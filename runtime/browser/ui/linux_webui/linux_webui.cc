// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/linux_webui/linux_webui.h"

#include "xwalk/runtime/browser/ui/linux_webui/select_file_dialog_impl_webui.h"

namespace views {

LinuxWebUI::LinuxWebUI() {
}

LinuxWebUI::~LinuxWebUI() {
}

ui::SelectFileDialog* LinuxWebUI::CreateSelectFileDialog(
    ui::SelectFileDialog::Listener* listener,
    ui::SelectFilePolicy* policy) const {
  return ui::SelectFileDialogImplWebUI::Create(listener, policy);
}

}  // namespace views

ui::LinuxShellDialog* BuildWebUI() {
  return new views::LinuxWebUI;
}
