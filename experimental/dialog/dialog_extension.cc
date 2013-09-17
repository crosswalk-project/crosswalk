// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/dialog/dialog_extension.h"

#include <utility>
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/jsapi/dialog.h"

using content::BrowserThread;

// This will be generated from dialog_api.js.
extern const char kSource_dialog_api[];

namespace xwalk {
namespace experimental {

using namespace jsapi::dialog; // NOLINT

DialogExtension::DialogExtension(RuntimeRegistry* runtime_registry)
  : XWalkInternalExtension(),
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

XWalkExtensionInstance* DialogExtension::CreateInstance() {
  return new DialogInstance(this);
}

void DialogExtension::OnRuntimeAdded(Runtime* runtime) {
  // FIXME(cmarcelo): We only support one runtime! (like MenuExtension)
  if (owning_window_)
    return;
  owning_window_ = runtime->window()->GetNativeWindow();
}

DialogInstance::DialogInstance(DialogExtension* extension)
  : extension_(extension),
    dialog_(NULL) {
  RegisterFunction("showOpenDialog", &DialogInstance::OnShowOpenDialog);
  RegisterFunction("showSaveDialog", &DialogInstance::OnShowSaveDialog);
}

DialogInstance::~DialogInstance() {
}

void DialogInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&XWalkInternalExtensionInstance::HandleMessage,
          base::Unretained(this), base::Passed(&msg)));
    return;
  }

  XWalkInternalExtensionInstance::HandleMessage(msg.Pass());
}

void DialogInstance::OnShowOpenDialog(const std::string& function_name,
                                     const std::string& callback_id,
                                     base::ListValue* args) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  scoped_ptr<ShowOpenDialog::Params>
      params(ShowOpenDialog::Params::Create(*args));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << function_name;
    return;
  }

  SelectFileDialog::Type dialog_type = SelectFileDialog::SELECT_OPEN_FILE;
  if (params->choose_directories)
    dialog_type = SelectFileDialog::SELECT_FOLDER;
  else if (params->allow_multiple_selection)
    dialog_type = SelectFileDialog::SELECT_OPEN_MULTI_FILE;

  string16 title16;
  UTF8ToUTF16(params->title.c_str(), params->title.length(), &title16);

  // FIXME(jeez): implement file_type and file_extension support.
  base::FilePath::StringType file_extension;

  std::pair<std::string, std::string>* data =
      new std::pair<std::string, std::string>(function_name, callback_id);

  if (!dialog_)
    dialog_ = ui::SelectFileDialog::Create(this, 0 /* policy */);

  dialog_->SelectFile(dialog_type, title16,
                      base::FilePath::FromUTF8Unsafe(params->initial_path),
                      NULL /* file_type */, 0 /* type_index */, file_extension,
                      extension_->owning_window_, data);
}

void DialogInstance::OnShowSaveDialog(const std::string& function_name,
                                     const std::string& callback_id,
                                     base::ListValue* args) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  scoped_ptr<ShowSaveDialog::Params>
      params(ShowSaveDialog::Params::Create(*args));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << function_name;
    return;
  }

  string16 title16;
  UTF8ToUTF16(params->title.c_str(), params->title.length(), &title16);

  // FIXME(jeez): implement file_type and file_extension support.
  base::FilePath::StringType file_extension;

  if (!dialog_)
    dialog_ = ui::SelectFileDialog::Create(this, 0 /* policy */);

  std::pair<std::string, std::string>* data =
      new std::pair<std::string, std::string>(function_name, callback_id);

  base::FilePath filePath =
      base::FilePath::FromUTF8Unsafe(params->initial_path);
  base::FilePath proposedFilePath =
      base::FilePath::FromUTF8Unsafe(params->proposed_new_filename);

  dialog_->SelectFile(SelectFileDialog::SELECT_SAVEAS_FILE, title16,
    filePath.Append(proposedFilePath), NULL /* file_type */, 0 /* type_index */,
    file_extension, extension_->owning_window_, data);
}

void DialogInstance::FileSelected(const base::FilePath& path, int,
                                 void* params) {
  scoped_ptr<std::pair<std::string, std::string> >
      data(static_cast<std::pair<std::string, std::string>*>(params));

  std::string strPath = path.AsUTF8Unsafe();
  if (data->first == "showOpenDialog") {
    std::vector<std::string> filesList;
    filesList.push_back(strPath);
    PostResult(data->second,
               ShowOpenDialog::Results::Create(filesList));
  } else {  // showSaveDialog
    PostResult(data->second,
               ShowSaveDialog::Results::Create(strPath));
  }
}

void DialogInstance::MultiFilesSelected(
    const std::vector<base::FilePath>& files, void* params) {
  scoped_ptr<std::pair<std::string, std::string> >
      data(static_cast<std::pair<std::string, std::string>*>(params));

  std::vector<std::string> filesList;
  std::vector<base::FilePath>::const_iterator it;
  for (it = files.begin(); it != files.end(); ++it)
    filesList.push_back(it->AsUTF8Unsafe());

  PostResult(data->second, ShowOpenDialog::Results::Create(filesList));
}

}  // namespace experimental
}  // namespace xwalk
