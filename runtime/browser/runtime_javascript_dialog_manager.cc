// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_javascript_dialog_manager.h"

#include <string>

#include "components/url_formatter/url_formatter.h"
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
#else
  *did_suppress_message = true;
  NOTIMPLEMENTED();
#endif
}

void RuntimeJavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    bool is_reload,
    const DialogClosedCallback& callback) {
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* bridge =
      XWalkContentsClientBridgeBase::FromWebContents(web_contents);
  bridge->RunBeforeUnloadDialog(web_contents->GetURL(),
                                callback);
#else
  NOTIMPLEMENTED();
  callback.Run(true, base::string16());
  return;
#endif
}

void RuntimeJavaScriptDialogManager::CancelActiveAndPendingDialogs(
    content::WebContents* web_contents) {
  NOTIMPLEMENTED();
}

void RuntimeJavaScriptDialogManager::ResetDialogState(
    content::WebContents* web_contents) {
  NOTIMPLEMENTED();
}

}  // namespace xwalk
