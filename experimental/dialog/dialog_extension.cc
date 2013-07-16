// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/dialog/dialog_extension.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"

using content::BrowserThread;

// This will be generated from dialog_api.js.
extern const char kSource_dialog_api[];

namespace xwalk {

DialogExtension::DialogExtension(RuntimeRegistry* runtime_registry)
  : XWalkExtension(),
    runtime_registry_(runtime_registry),
    owning_window_(NULL) {
  set_name("xwalk.experimental.dialog");
  runtime_registry_->AddObserver(this);
}

DialogExtension::~DialogExtension() {
  runtime_registry_->RemoveObserver(this);
}

const char* DialogExtension::GetJavaScriptAPI() {
  return kSource_dialog_api;
}

XWalkExtension::Context* DialogExtension::CreateContext(
  const XWalkExtension::PostMessageCallback& post_message) {
  return new DialogContext(this, post_message);
}

void DialogExtension::OnRuntimeAdded(Runtime* runtime) {
  // FIXME(cmarcelo): We only support one runtime! (like MenuExtension)
  if (owning_window_)
    return;
  owning_window_ = runtime->window()->GetNativeWindow();
}

DialogContext::DialogContext(DialogExtension* extension,
  const XWalkExtension::PostMessageCallback& post_message)
  : XWalkExtension::Context(post_message),
    extension_(extension),
    dialog_(NULL) {
}

DialogContext::~DialogContext() {
}

void DialogContext::HandleShowOpenDialog(const base::DictionaryValue* input) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  std::string reply_id;
  input->GetString("_reply_id", &reply_id);
  std::string* reply_id_ptr = new std::string(reply_id);

  bool is_multiple_selection = false;
  input->GetBoolean("allow_multiple_selection", &is_multiple_selection);

  bool is_choose_directory = false;
  input->GetBoolean("choose_directory", &is_choose_directory);

  std::string title;
  input->GetString("title", &title);
  string16 title16;
  UTF8ToUTF16(title.c_str(), title.length(), &title16);

  base::FilePath::StringType initial_path;
  input->GetString("title", &initial_path);

  base::FilePath::StringType file_extension;

  SelectFileDialog::Type dialog_type = SelectFileDialog::SELECT_OPEN_FILE;
  if (is_choose_directory)
    dialog_type = SelectFileDialog::SELECT_FOLDER;
  else if (is_multiple_selection)
    dialog_type = SelectFileDialog::SELECT_OPEN_MULTI_FILE;

  if (!dialog_)
    dialog_ = ui::SelectFileDialog::Create(this, 0 /* policy */);

  // FIXME(jeez): implement file_type and file_extension support.
  dialog_->SelectFile(dialog_type, title16, base::FilePath(initial_path),
    NULL /* file_type */, 0 /* type_index */, file_extension,
    extension_->owning_window_, reply_id_ptr);
}

void DialogContext::HandleShowSaveDialog(const base::DictionaryValue* input) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  std::string reply_id;
  input->GetString("_reply_id", &reply_id);
  std::string* reply_id_ptr = new std::string(reply_id);

  std::string title;
  input->GetString("title", &title);
  string16 title16;
  UTF8ToUTF16(title.c_str(), title.length(), &title16);

  base::FilePath::StringType initial_path;
  input->GetString("initial_path", &initial_path);

  base::FilePath::StringType proposed_filename;
  input->GetString("proposed_name", &proposed_filename);

  base::FilePath::StringType file_extension;

  if (!dialog_)
    dialog_ = ui::SelectFileDialog::Create(this, 0 /* policy */);

  dialog_->SelectFile(SelectFileDialog::SELECT_SAVEAS_FILE, title16,
    base::FilePath(initial_path).Append(proposed_filename),
    NULL /* file_type */, 0 /* type_index */, file_extension,
    extension_->owning_window_, reply_id_ptr);
}

void DialogContext::HandleMessage(const std::string& msg) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&DialogContext::HandleMessage, base::Unretained(this), msg));
    return;
  }

  scoped_ptr<base::Value> v(base::JSONReader().ReadToValue(msg));
  const base::DictionaryValue* input = static_cast<base::DictionaryValue*>(
    v.get());

  std::string cmd;
  input->GetString("cmd", &cmd);

  if (cmd == "ShowOpenDialog")
    HandleShowOpenDialog(input);
  else if (cmd == "ShowSaveDialog")
    HandleShowSaveDialog(input);
}

void DialogContext::FileSelected(const base::FilePath& path,
  int index, void* params) {
  std::string* reply_id = static_cast<std::string*>(params);
  scoped_ptr<base::DictionaryValue> output(new base::DictionaryValue);
  output->SetString("_reply_id", *reply_id);
  output->SetString("file", path.value());

  std::string result;
  base::JSONWriter::Write(output.get(), &result);
  PostMessage(result);

  delete reply_id;
}

void DialogContext::MultiFilesSelected(
  const std::vector<base::FilePath>& files, void* params) {
  std::string* reply_id = static_cast<std::string*>(params);
  scoped_ptr<base::DictionaryValue> output(new base::DictionaryValue);
  output->SetString("_reply_id", *reply_id);

  base::ListValue* filesList = new base::ListValue;
  std::vector<base::FilePath>::const_iterator it;
  for (it = files.begin(); it != files.end(); ++it)
    filesList->AppendString(it->value());

  output->Set("file", filesList);

  std::string result;
  base::JSONWriter::Write(output.get(), &result);
  PostMessage(result);

  delete reply_id;
}

}  // namespace xwalk
