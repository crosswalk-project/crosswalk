// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell.h"

#include <commctrl.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

#include "base/command_line.h"
#include "base/utf_string_conversions.h"
#include "base/win/wrapped_window_proc.h"
#include "cameo/src/browser/resource.h"
#include "cameo/src/browser/ui/native_app_window_win.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "ui/base/win/hwnd_util.h"

namespace cameo {

HINSTANCE Shell::instance_handle_;

void Shell::PlatformInitialize(const gfx::Size& default_window_size) {
}

void Shell::PlatformExit() {
  std::vector<Shell*> windows = windows_;
  for (std::vector<Shell*>::iterator it = windows.begin();
       it != windows.end(); ++it)
    (*it)->window_->Close();
}

void Shell::PlatformCleanUp() {
  // When the window is destroyed, tell the Edit field to forget about us,
  // otherwise we will crash.
  if (headless_)
    return;
}

void Shell::PlatformEnableUIControl(UIControl control, bool is_enabled) {
  if (headless_)
    return;

  // int id;
  // switch (control) {
  //   case BACK_BUTTON:
  //     id = IDC_NAV_BACK;
  //     break;
  //   case FORWARD_BUTTON:
  //     id = IDC_NAV_FORWARD;
  //     break;
  //   case STOP_BUTTON:
  //     id = IDC_NAV_STOP;
  //     break;
  //   default:
  //     NOTREACHED() << "Unknown UI control";
  //     return;
  // }
  // EnableWindow(GetDlgItem(window_, id), is_enabled);
}

void Shell::PlatformSetAddressBarURL(const GURL& url) {
  if (headless_)
    return;

  window_->SetToolbarUrlEntry(url.spec());
}

void Shell::PlatformSetIsLoading(bool loading) {
  window_->SetToolbarIsLoading(loading);
}

void Shell::PlatformCreateWindow(int width, int height) {
  NativeAppWindow::CreateParams params;
  params.size.set_width(width);
  params.size.set_height(height);
  window_ = NativeAppWindow::Create(this, params);
}

void Shell::PlatformSetContents() {
}

void Shell::SizeTo(int width, int height) {
  window_->SetBounds(gfx::Rect(0, 0, width, height));
}

void Shell::PlatformResizeSubViews() {
}

void Shell::Close() {
  window_->Close();
}

void Shell::PlatformSetTitle(const string16& text) {
  window_->SetTitle(UTF16ToUTF8(text));
}

}  // namespace cameo
