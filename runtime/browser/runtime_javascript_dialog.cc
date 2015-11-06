// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_javascript_dialog.h"

#if defined(OS_WIN)
#include "xwalk/runtime/browser/runtime_javascript_dialog_win.h"
#endif

namespace xwalk {

scoped_ptr<RuntimeJavaScriptDialog> RuntimeJavaScriptDialog::Create(
  RuntimeJavaScriptDialogManager* manager,
  gfx::NativeWindow parent_window,
  content::JavaScriptMessageType message_type,
  const base::string16& message_text,
  const base::string16& default_prompt_text,
  const content::JavaScriptDialogManager::DialogClosedCallback& callback) {
#if defined(OS_WIN)
  scoped_ptr<RuntimeJavaScriptDialog> dialog(new RuntimeJavaScriptDialogWin(
      manager,
      parent_window,
      message_type,
      message_text,
      default_prompt_text,
      callback));
  return dialog.Pass();
#else
  return scoped_ptr<RuntimeJavaScriptDialog>();
#endif
}

RuntimeJavaScriptDialog::RuntimeJavaScriptDialog() {
}

RuntimeJavaScriptDialog::~RuntimeJavaScriptDialog() {
  Cancel();
}

}  // namespace xwalk
