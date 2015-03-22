// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/linux_webui/select_file_dialog_impl_webui.h"

#include <deque>
#include <string>

#include "base/environment.h"
#include "base/files/file_util.h"
#include "base/lazy_instance.h"
#include "base/nix/xdg_util.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/web_contents.h"
#include "xwalk/runtime/browser/ui/webui/file_picker/file_picker_web_dialog.h"

namespace ui {

base::FilePath* SelectFileDialogImplWebUI::last_saved_path_ = NULL;
base::FilePath* SelectFileDialogImplWebUI::last_opened_path_ = NULL;

// static
ui::SelectFileDialog* SelectFileDialogImplWebUI::Create(
    SelectFileDialog::Listener* listener,
    SelectFilePolicy* policy) {
  return new SelectFileDialogImplWebUI(listener, policy);
}

SelectFileDialogImplWebUI::SelectFileDialogImplWebUI(Listener* listener,
                                           ui::SelectFilePolicy* policy)
    : SelectFileDialog(listener, policy),
      file_type_index_(0),
      type_(SELECT_NONE) {
  if (!last_saved_path_) {
    last_saved_path_ = new base::FilePath();
    last_opened_path_ = new base::FilePath();
  }
}

SelectFileDialogImplWebUI::~SelectFileDialogImplWebUI() { }

bool SelectFileDialogImplWebUI::HasMultipleFileTypeChoicesImpl() {
  NOTIMPLEMENTED();
  return false;
}

bool SelectFileDialogImplWebUI::IsRunning(gfx::NativeWindow parent_window)
    const {
  NOTIMPLEMENTED();
  return false;
}
void SelectFileDialogImplWebUI::ListenerDestroyed() {
  listener_ = NULL;
}

void SelectFileDialogImplWebUI::SelectFileImpl(
    SelectFileDialog::Type type,
    const base::string16& title,
    const base::FilePath& default_path,
    const SelectFileDialog::FileTypeInfo* file_types,
    int file_type_index,
    const std::string& default_extension,
    gfx::NativeWindow owning_window,
    void* params) {

  content::WebContents* web_contents =
      static_cast<content::WebContents*>(params);

  // FIXME(joone): Title, default_path, and file_types should be passed
  //               to ShowDialog.
  FilePickerWebDialog::ShowDialog(type, owning_window, web_contents, listener_);
}

bool SelectFileDialogImplWebUI::CallDirectoryExistsOnUIThread(
    const base::FilePath& path) {
  base::ThreadRestrictions::ScopedAllowIO allow_io;
  return base::DirectoryExists(path);
}

}  // namespace ui
