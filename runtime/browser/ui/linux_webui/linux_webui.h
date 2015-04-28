// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_LINUX_WEBUI_LINUX_WEBUI_H_
#define XWALK_RUNTIME_BROWSER_UI_LINUX_WEBUI_LINUX_WEBUI_H_

#include "ui/shell_dialogs/linux_shell_dialog.h"

namespace views {

class LinuxWebUI : public ui::LinuxShellDialog {
 public:
  LinuxWebUI();
  ~LinuxWebUI() override;

  // ui::LinuxShellDialog:
  ui::SelectFileDialog* CreateSelectFileDialog(
      ui::SelectFileDialog::Listener* listener,
      ui::SelectFilePolicy* policy) const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(LinuxWebUI);
};

}  // namespace views

ui::LinuxShellDialog* BuildWebUI();

#endif  // XWALK_RUNTIME_BROWSER_UI_LINUX_WEBUI_LINUX_WEBUI_H_
