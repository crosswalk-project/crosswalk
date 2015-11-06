// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_JAVASCRIPT_DIALOG_WIN_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_JAVASCRIPT_DIALOG_WIN_H_

#include "xwalk/runtime/browser/runtime_javascript_dialog.h"

namespace xwalk {

class RuntimeJavaScriptDialogWin : public RuntimeJavaScriptDialog {
 public:
  RuntimeJavaScriptDialogWin(
    RuntimeJavaScriptDialogManager* manager,
    gfx::NativeWindow parent_window,
    content::JavaScriptMessageType message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback);

  void Cancel() override;

 private:
  content::JavaScriptMessageType message_type_;
  HWND dialog_win_;
  base::string16 message_text_;
  base::string16 default_prompt_text_;
  static INT_PTR CALLBACK DialogProc(HWND dialog, UINT message, WPARAM wparam,
                                     LPARAM lparam);

  DISALLOW_COPY_AND_ASSIGN(RuntimeJavaScriptDialogWin);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_JAVASCRIPT_DIALOG_WIN_H_
