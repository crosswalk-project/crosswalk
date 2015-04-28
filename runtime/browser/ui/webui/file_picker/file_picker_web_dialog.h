// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_WEBUI_FILE_PICKER_FILE_PICKER_WEB_DIALOG_H_
#define XWALK_RUNTIME_BROWSER_UI_WEBUI_FILE_PICKER_FILE_PICKER_WEB_DIALOG_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "ui/web_dialogs/web_dialog_delegate.h"

namespace ui {

// Launches a web dialog for file picker with specified URL and title.
class FilePickerWebDialog : public ui::WebDialogDelegate {
 public:
  // Shows the dialog box.
  static void ShowDialog(SelectFileDialog::Type type,
                         gfx::NativeWindow owning_window,
                         content::WebContents* contents,
                         SelectFileDialog::Listener* listener);
  // Closes the dialog, which will delete itself.
  void Close(const std::string& file_path) const;

 private:
  FilePickerWebDialog(SelectFileDialog::Type type, SelectFileDialog::Listener*);

  // Overridden from ui::WebDialogDelegate:
  ui::ModalType GetDialogModalType() const override;
  base::string16 GetDialogTitle() const override;
  GURL GetDialogContentURL() const override;
  void GetWebUIMessageHandlers(
      std::vector<content::WebUIMessageHandler*>* handlers) const override;
  void GetDialogSize(gfx::Size* size) const override;
  std::string GetDialogArgs() const override;
  void OnDialogClosed(const std::string& json_retval) override;
  void OnCloseContents(content::WebContents* source,
                       bool* out_close_dialog) override;
  bool ShouldShowDialogTitle() const override;
  bool HandleContextMenu(const content::ContextMenuParams& params) override;

  SelectFileDialog::Type type_;
  SelectFileDialog::Listener* listener_;

  DISALLOW_COPY_AND_ASSIGN(FilePickerWebDialog);
};

}  // namespace ui


#endif  // XWALK_RUNTIME_BROWSER_UI_WEBUI_FILE_PICKER_FILE_PICKER_WEB_DIALOG_H_
