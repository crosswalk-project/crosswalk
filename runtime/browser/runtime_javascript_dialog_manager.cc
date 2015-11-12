// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_javascript_dialog_manager.h"

#include <string>

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_util.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#endif  // defined(OS_ANDROID)

#if defined(OS_WIN)
#include "xwalk/runtime/browser/runtime_javascript_dialog.h"
#endif

namespace xwalk {

RuntimeJavaScriptDialogManager::RuntimeJavaScriptDialogManager() {
}

RuntimeJavaScriptDialogManager::~RuntimeJavaScriptDialogManager() {
}

void RuntimeJavaScriptDialogManager::RunJavaScriptDialog(
    content::WebContents* web_contents,
    const GURL& origin_url,
    const std::string& accept_lang,
    content::JavaScriptMessageType javascript_message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) {
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* bridge =
      XWalkContentsClientBridgeBase::FromWebContents(web_contents);
  bridge->RunJavaScriptDialog(javascript_message_type,
                              origin_url,
                              message_text,
                              default_prompt_text,
                              callback);
#elif defined(OS_WIN)
  *did_suppress_message = false;

  if (dialog_) {
    // One dialog at a time.
    *did_suppress_message = true;
    return;
  }

  base::string16 new_message_text = net::FormatUrl(origin_url, accept_lang) +
      base::ASCIIToUTF16("\n\n") +
      message_text;
  gfx::NativeWindow parent_window = web_contents->GetTopLevelNativeWindow();

  dialog_ = RuntimeJavaScriptDialog::Create(this,
      parent_window,
      javascript_message_type,
      new_message_text,
      default_prompt_text,
      callback);
#else
  *did_suppress_message = true;
  NOTIMPLEMENTED();
#endif
}

void RuntimeJavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    const base::string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) {
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* bridge =
      XWalkContentsClientBridgeBase::FromWebContents(web_contents);
  bridge->RunBeforeUnloadDialog(web_contents->GetURL(),
                                message_text,
                                callback);
#elif defined(OS_WIN)
  if (dialog_) {
    callback.Run(true, base::string16());
    return;
  }

  base::string16 new_message_text =
      message_text +
      base::ASCIIToUTF16("\n\nIs it OK to leave/reload this page?");

  gfx::NativeWindow parent_window = web_contents->GetTopLevelNativeWindow();

  dialog_ = RuntimeJavaScriptDialog::Create(this,
      parent_window,
      content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM,
      new_message_text,
      base::string16(),  // default
      callback);
#else
  NOTIMPLEMENTED();
  callback.Run(true, base::string16());
  return;
#endif
}

void RuntimeJavaScriptDialogManager::CancelActiveAndPendingDialogs(
    content::WebContents* web_contents) {
#if defined(OS_WIN)
  if (dialog_) {
    dialog_->Cancel();
    dialog_.reset();
  }
#else
  NOTIMPLEMENTED();
#endif
}

void RuntimeJavaScriptDialogManager::ResetDialogState(
    content::WebContents* web_contents) {
}

void RuntimeJavaScriptDialogManager::DialogClosed(
    RuntimeJavaScriptDialog* dialog) {
#if defined(OS_WIN)
  DCHECK_EQ(dialog, dialog_.get());
  dialog_.reset();
#endif
}

}  // namespace xwalk
