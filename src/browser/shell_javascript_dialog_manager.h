// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_JAVASCRIPT_DIALOG_MANAGER_H_
#define CAMEO_SRC_BROWSER_SHELL_JAVASCRIPT_DIALOG_MANAGER_H_

#include <string>

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/javascript_dialog_manager.h"

namespace cameo {

class ShellJavaScriptDialog;

class ShellJavaScriptDialogManager : public content::JavaScriptDialogManager {
 public:
  ShellJavaScriptDialogManager();
  virtual ~ShellJavaScriptDialogManager();

  // JavaScriptDialogManager:
  virtual void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType javascript_message_type,
      const string16& message_text,
      const string16& default_prompt_text,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) OVERRIDE;

  virtual void RunBeforeUnloadDialog(
      content::WebContents* web_contents,
      const string16& message_text,
      bool is_reload,
      const DialogClosedCallback& callback) OVERRIDE;

  virtual void ResetJavaScriptState(
      content::WebContents* web_contents) OVERRIDE;

  // Called by the ShellJavaScriptDialog when it closes.
  void DialogClosed(ShellJavaScriptDialog* dialog);

  // Used for content_browsertests.
  void set_dialog_request_callback(const base::Closure& callback) {
    dialog_request_callback_ = callback;
  }

 private:
#if defined(OS_MACOSX) || defined(OS_WIN) || defined(TOOLKIT_GTK)
  // The dialog being shown. No queueing.
  scoped_ptr<ShellJavaScriptDialog> dialog_;
#else
  // TODO(nhu): implement ShellJavaScriptDialog for other platforms,
  // drop this #if
#endif

  base::Closure dialog_request_callback_;

  DISALLOW_COPY_AND_ASSIGN(ShellJavaScriptDialogManager);
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_SHELL_JAVASCRIPT_DIALOG_MANAGER_H_
