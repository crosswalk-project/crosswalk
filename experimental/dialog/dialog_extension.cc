// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/dialog/dialog_extension.h"

#include <utility>
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "grit/xwalk_experimental_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/experimental/dialog/dialog.h"

using content::BrowserThread;

namespace xwalk {
namespace experimental {

using namespace jsapi::dialog; // NOLINT

DialogExtension::DialogExtension(RuntimeRegistry* runtime_registry)
  : runtime_registry_(runtime_registry),
    owning_window_(NULL) {
  set_name("xwalk.experimental.dialog");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_EXPERIMENTAL_DIALOG_API).as_string());
  runtime_registry_->AddObserver(this);
}

DialogExtension::~DialogExtension() {
  runtime_registry_->RemoveObserver(this);
}

XWalkExtensionInstance* DialogExtension::CreateInstance() {
  return new DialogInstance(this);
}

void DialogExtension::OnRuntimeAdded(Runtime* runtime) {
  // FIXME(cmarcelo): We only support one runtime! (like MenuExtension)
  if (owning_window_)
    return;
  if (runtime->window())
    owning_window_ = runtime->window()->GetNativeWindow();
}

DialogInstance::DialogInstance(DialogExtension* extension)
  : extension_(extension),
    dialog_(NULL),
    handler_(this) {
  handler_.Register("showOpenDialog",
      base::Bind(&DialogInstance::OnShowOpenDialog, base::Unretained(this)));
  handler_.Register("showSaveDialog",
      base::Bind(&DialogInstance::OnShowSaveDialog, base::Unretained(this)));
}

DialogInstance::~DialogInstance() {
}

void DialogInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&DialogInstance::HandleMessage,
          base::Unretained(this), base::Passed(&msg)));
    return;
  }

  handler_.HandleMessage(msg.Pass());
}

void DialogInstance::OnShowOpenDialog(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  scoped_ptr<ShowOpenDialog::Params>
      params(ShowOpenDialog::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
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

  if (!dialog_)
    dialog_ = ui::SelectFileDialog::Create(this, 0 /* policy */);

  dialog_->SelectFile(dialog_type, title16,
                      base::FilePath::FromUTF8Unsafe(params->initial_path),
                      NULL /* file_type */, 0 /* type_index */, file_extension,
                      extension_->owning_window_, info.release());
}

void DialogInstance::OnShowSaveDialog(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  scoped_ptr<ShowSaveDialog::Params>
      params(ShowSaveDialog::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  string16 title16;
  UTF8ToUTF16(params->title.c_str(), params->title.length(), &title16);

  // FIXME(jeez): implement file_type and file_extension support.
  base::FilePath::StringType file_extension;

  if (!dialog_)
    dialog_ = ui::SelectFileDialog::Create(this, 0 /* policy */);

  base::FilePath filePath =
      base::FilePath::FromUTF8Unsafe(params->initial_path);
  base::FilePath proposedFilePath =
      base::FilePath::FromUTF8Unsafe(params->proposed_new_filename);

  dialog_->SelectFile(SelectFileDialog::SELECT_SAVEAS_FILE, title16,
    filePath.Append(proposedFilePath), NULL /* file_type */, 0 /* type_index */,
    file_extension, extension_->owning_window_, info.release());
}

void DialogInstance::FileSelected(const base::FilePath& path, int,
                                 void* params) {
  scoped_ptr<XWalkExtensionFunctionInfo> info(
      static_cast<XWalkExtensionFunctionInfo*>(params));

  std::string strPath = path.AsUTF8Unsafe();
  if (info->name() == "showOpenDialog") {
    std::vector<std::string> filesList;
    filesList.push_back(strPath);
    info->PostResult(ShowOpenDialog::Results::Create(filesList));
  } else {  // showSaveDialog
    info->PostResult(ShowSaveDialog::Results::Create(strPath));
  }
}

void DialogInstance::MultiFilesSelected(
    const std::vector<base::FilePath>& files, void* params) {
  scoped_ptr<XWalkExtensionFunctionInfo> info(
      static_cast<XWalkExtensionFunctionInfo*>(params));

  std::vector<std::string> filesList;
  std::vector<base::FilePath>::const_iterator it;
  for (it = files.begin(); it != files.end(); ++it)
    filesList.push_back(it->AsUTF8Unsafe());

  info->PostResult(ShowOpenDialog::Results::Create(filesList));
}

}  // namespace experimental
}  // namespace xwalk
