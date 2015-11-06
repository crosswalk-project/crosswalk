// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_javascript_dialog_win.h"

#include "base/strings/string_util.h"
#include "xwalk/runtime/resources/resource.h"
#include "xwalk/runtime/browser/runtime_javascript_dialog_manager.h"

namespace xwalk {

INT_PTR CALLBACK RuntimeJavaScriptDialogWin::DialogProc(HWND dialog,
                                                    UINT message,
                                                    WPARAM wparam,
                                                    LPARAM lparam) {
  switch (message) {
    case WM_INITDIALOG: {
      SetWindowLongPtr(dialog, DWLP_USER, static_cast<LONG_PTR>(lparam));
      RuntimeJavaScriptDialogWin* owner =
          reinterpret_cast<RuntimeJavaScriptDialogWin*>(lparam);
      owner->dialog_win_ = dialog;
      SetDlgItemText(dialog, IDC_DIALOGTEXT, owner->message_text_.c_str());
      if (owner->message_type_ == content::JAVASCRIPT_MESSAGE_TYPE_PROMPT)
        SetDlgItemText(dialog, IDC_PROMPTEDIT,
                       owner->default_prompt_text_.c_str());
      break;
    }
    case WM_DESTROY: {
      RuntimeJavaScriptDialogWin* owner =
          reinterpret_cast<RuntimeJavaScriptDialogWin*>(
              GetWindowLongPtr(dialog, DWLP_USER));
      if (owner->dialog_win_) {
        owner->dialog_win_ = 0;
        owner->callback_.Run(false, base::string16());
        owner->manager_->DialogClosed(owner);
      }
      break;
    }
    case WM_COMMAND: {
      RuntimeJavaScriptDialogWin* owner =
          reinterpret_cast<RuntimeJavaScriptDialogWin*>(
              GetWindowLongPtr(dialog, DWLP_USER));
      base::string16 user_input;
      bool finish = false;
      bool result;
      switch (LOWORD(wparam)) {
        case IDOK:
          finish = true;
          result = true;
          if (owner->message_type_ ==
                content::JAVASCRIPT_MESSAGE_TYPE_PROMPT) {
            int length =
                GetWindowTextLength(GetDlgItem(dialog, IDC_PROMPTEDIT)) + 1;
            GetDlgItemText(dialog, IDC_PROMPTEDIT,
                           base::WriteInto(&user_input, length), length);
          }
          break;
        case IDCANCEL:
          finish = true;
          result = false;
          break;
      }
      if (finish) {
        owner->dialog_win_ = 0;
        owner->callback_.Run(result, user_input);
        DestroyWindow(dialog);
        owner->manager_->DialogClosed(owner);
      }
      break;
    }
    default:
      return DefWindowProc(dialog, message, wparam, lparam);
  }
  return 0;
}

RuntimeJavaScriptDialogWin::RuntimeJavaScriptDialogWin(
    RuntimeJavaScriptDialogManager* manager,
    gfx::NativeWindow parent_window,
    content::JavaScriptMessageType message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback)
    : message_type_(message_type),
      message_text_(message_text),
      default_prompt_text_(default_prompt_text) {
  manager_ = manager;
  callback_ = callback;
  int dialog_type;
  if (message_type == content::JAVASCRIPT_MESSAGE_TYPE_ALERT)
    dialog_type = IDD_ALERT;
  else if (message_type == content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM)
    dialog_type = IDD_CONFIRM;
  else  // JAVASCRIPT_MESSAGE_TYPE_PROMPT
    dialog_type = IDD_PROMPT;

  dialog_win_ = CreateDialogParam(GetModuleHandle(0),
                                  MAKEINTRESOURCE(dialog_type), 0, DialogProc,
                                  reinterpret_cast<LPARAM>(this));
  ShowWindow(dialog_win_, SW_SHOWNORMAL);
}

void RuntimeJavaScriptDialogWin::Cancel() {
  if (dialog_win_)
    DestroyWindow(dialog_win_);
}

}  // namespace xwalk
