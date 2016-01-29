// Copyright 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_MODAL_DIALOG_H_
#define XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_MODAL_DIALOG_H_

#include <map>
#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "components/app_modal/app_modal_dialog.h"

namespace xwalk {

class XWalkPermissionModalDialog : public app_modal::AppModalDialog {
 public:
  XWalkPermissionModalDialog(
      content::WebContents* web_contents,
      const base::string16& message_text,
      const base::Callback<void(bool)>& callback);
  ~XWalkPermissionModalDialog() override;

  // Overridden from AppModalDialog:
  app_modal::NativeAppModalDialog* CreateNativeDialog() override;
  void Invalidate() override;

  // Callbacks from NativeDialog when the user accepts or cancels the dialog.
  void OnCancel(bool suppress_js_messages);
  void OnAccept(const base::string16& prompt_text, bool suppress_js_messages);

  // NOTE: This is only called under Views, and should be removed. Any critical
  // work should be done in OnCancel or OnAccept. See crbug.com/63732 for more.
  void OnClose();

  // Accessors
  base::string16 message_text() const { return message_text_; }

 private:
  // Notifies the delegate with the result of the dialog.
  void NotifyDelegate(bool success);

  base::string16 message_text_;

  base::Callback<void(bool)> callback_;

  DISALLOW_COPY_AND_ASSIGN(XWalkPermissionModalDialog);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_UI_DESKTOP_XWALK_PERMISSION_MODAL_DIALOG_H_
