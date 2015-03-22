// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file implements select dialog functionality using webview.

#ifndef XWALK_RUNTIME_BROWSER_UI_LINUX_WEBUI_SELECT_FILE_DIALOG_IMPL_WEBUI_H_
#define XWALK_RUNTIME_BROWSER_UI_LINUX_WEBUI_SELECT_FILE_DIALOG_IMPL_WEBUI_H_

#include <set>

#include "base/compiler_specific.h"
#include "base/nix/xdg_util.h"
#include "ui/shell_dialogs/select_file_dialog.h"

namespace ui {

class SelectFileDialogImplWebUI : public SelectFileDialog {
 public:
  // Main factory method which returns correct type.
  static ui::SelectFileDialog* Create(Listener* listener,
                                      ui::SelectFilePolicy* policy);
  // BaseShellDialog implementation.
  bool IsRunning(gfx::NativeWindow parent_window) const override;
  void ListenerDestroyed() override;

 protected:
  explicit SelectFileDialogImplWebUI(Listener* listener,
                                     ui::SelectFilePolicy* policy);
  ~SelectFileDialogImplWebUI() override;

  // SelectFileDialog implementation.
  // |params| is user data we pass back via the Listener interface.
  void SelectFileImpl(Type type,
                      const base::string16& title,
                      const base::FilePath& default_path,
                      const FileTypeInfo* file_types,
                      int file_type_index,
                      const base::FilePath::StringType& default_extension,
                      gfx::NativeWindow owning_window,
                      void* params) override;
  bool HasMultipleFileTypeChoicesImpl() override;

  // Wrapper for base::DirectoryExists() that allow access on the UI
  // thread. Use this only in the file dialog functions, where it's ok
  // because the file dialog has to do many stats anyway. One more won't
  // hurt too badly and it's likely already cached.
  bool CallDirectoryExistsOnUIThread(const base::FilePath& path);

  // The file filters.
  FileTypeInfo file_types_;

  // The index of the default selected file filter.
  // Note: This starts from 1, not 0.
  size_t file_type_index_;

  // The type of dialog we are showing the user.
  Type type_;

  // These two variables track where the user last saved a file or opened a
  // file so that we can display future dialogs with the same starting path.
  static base::FilePath* last_saved_path_;
  static base::FilePath* last_opened_path_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SelectFileDialogImplWebUI);
};

SelectFileDialog* CreateLinuxSelectFileDialog(
    SelectFileDialog::Listener* listener,
    SelectFilePolicy* policy);

}  // namespace ui

#endif  // XWALK_RUNTIME_BROWSER_UI_LINUX_WEBUI_SELECT_FILE_DIALOG_IMPL_WEBUI_H_
