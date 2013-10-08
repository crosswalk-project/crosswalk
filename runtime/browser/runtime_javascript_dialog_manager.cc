// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_javascript_dialog_manager.h"

#include <string>

#include "content/public/browser/javascript_dialog_manager.h"
#include "content/public/browser/web_contents.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#endif  // defined(OS_ANDROID)

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
    const string16& message_text,
    const string16& default_prompt_text,
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
#else
  NOTIMPLEMENTED();
#endif
}

void RuntimeJavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    const string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) {
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* bridge =
      XWalkContentsClientBridgeBase::FromWebContents(web_contents);
  bridge->RunBeforeUnloadDialog(web_contents->GetURL(),
                                message_text,
                                callback);
#else
  NOTIMPLEMENTED();
#endif
}

void RuntimeJavaScriptDialogManager::CancelActiveAndPendingDialogs(
    content::WebContents* web_contents) {
  NOTIMPLEMENTED();
}

void RuntimeJavaScriptDialogManager::WebContentsDestroyed(
    content::WebContents* web_contents) {
  NOTIMPLEMENTED();
}

}  // namespace xwalk
