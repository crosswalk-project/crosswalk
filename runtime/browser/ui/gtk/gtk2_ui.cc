// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtk/gtk.h>

#include "xwalk/runtime/browser/ui/gtk/gtk2_ui.h"
#include "xwalk/runtime/browser/ui/gtk/select_file_dialog_gtk2.h"

namespace {

ui::LinuxShellDialog* g_gtk_dialogs = nullptr;

}  // namespace

namespace xwalk {

Gtk2UI::Gtk2UI() {
  gtk_init(0, nullptr);
}

Gtk2UI::~Gtk2UI() {
}

ui::SelectFileDialog* Gtk2UI::CreateSelectFileDialog(
    ui::SelectFileDialog::Listener* listener,
    ui::SelectFilePolicy* policy) const {
  return SelectFileDialogGTK2::Create(listener, policy);
}

ui::LinuxShellDialog* Gtk2Dialogs() {
  if (g_gtk_dialogs)
    return g_gtk_dialogs;

  return g_gtk_dialogs = new Gtk2UI();
}

}  // namespace xwalk
