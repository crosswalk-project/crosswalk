// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_JAVASCRIPT_DIALOG_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_JAVASCRIPT_DIALOG_H_

#include "xwalk/runtime/browser/runtime_javascript_dialog_manager.h"

namespace xwalk {

class RuntimeJavaScriptDialogManager;

class RuntimeJavaScriptDialog {
 public:
  static scoped_ptr<RuntimeJavaScriptDialog> Create(
    RuntimeJavaScriptDialogManager* manager,
    gfx::NativeWindow parent_window,
    content::JavaScriptMessageType message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback);

  virtual void Cancel() {}
  virtual ~RuntimeJavaScriptDialog();
 protected:
  RuntimeJavaScriptDialog();
  RuntimeJavaScriptDialogManager* manager_;
  RuntimeJavaScriptDialogManager::DialogClosedCallback callback_;
 private:
  DISALLOW_COPY_AND_ASSIGN(RuntimeJavaScriptDialog);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_JAVASCRIPT_DIALOG_H_
