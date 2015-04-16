// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_GTK_GTK2_UI_H_
#define XWALK_RUNTIME_BROWSER_UI_GTK_GTK2_UI_H_

#include "ui/shell_dialogs/linux_shell_dialog.h"

namespace xwalk {

// Factory class to create GTK dialogs.
class Gtk2UI : public ui::LinuxShellDialog {
 public:
  Gtk2UI();
  ~Gtk2UI() override;

  ui::SelectFileDialog* CreateSelectFileDialog(
      ui::SelectFileDialog::Listener* listener,
      ui::SelectFilePolicy* policy) const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(Gtk2UI);
};

// Return the unique instance of class 'Gtk2UI'.
ui::LinuxShellDialog* Gtk2Dialogs();

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_GTK_GTK2_UI_H_
