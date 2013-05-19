// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_JAVASCRIPT_APP_MODAL_DIALOG_H_
#define CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_JAVASCRIPT_APP_MODAL_DIALOG_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/time.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/app_modal_dialog.h"
#include "content/public/browser/javascript_dialog_manager.h"

// Extra data for JavaScript dialogs to add Runtime-only features.
class RuntimeJavaScriptDialogExtraData {
 public:
  RuntimeJavaScriptDialogExtraData();

  // The time that the last JavaScript dialog was dismissed.
  base::TimeTicks last_javascript_message_dismissal_;

  // True if the user has decided to block future JavaScript dialogs.
  bool suppress_javascript_messages_;
};

// A controller + model class for JavaScript alert, confirm, prompt, and
// onbeforeunload dialog boxes.
class JavaScriptAppModalDialog : public AppModalDialog {
 public:
  JavaScriptAppModalDialog(
      content::WebContents* web_contents,
      RuntimeJavaScriptDialogExtraData* extra_data,
      const string16& title,
      content::JavaScriptMessageType javascript_message_type,
      const string16& message_text,
      const string16& default_prompt_text,
      bool display_suppress_checkbox,
      bool is_before_unload_dialog,
      bool is_reload,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback);
  virtual ~JavaScriptAppModalDialog();

  // Overridden from AppModalDialog:
  virtual NativeAppModalDialog* CreateNativeDialog() OVERRIDE;
  virtual bool IsJavaScriptModalDialog() OVERRIDE;
  virtual void Invalidate() OVERRIDE;

  // Callbacks from NativeDialog when the user accepts or cancels the dialog.
  void OnCancel(bool suppress_js_messages);
  void OnAccept(const string16& prompt_text, bool suppress_js_messages);

  // NOTE: This is only called under Views, and should be removed. Any critical
  // work should be done in OnCancel or OnAccept.
  void OnClose();

  // Used only for testing. The dialog will use the given text when notifying
  // its delegate instead of whatever the UI reports.
  void SetOverridePromptText(const string16& prompt_text);

  // Accessors
  content::JavaScriptMessageType javascript_message_type() const {
    return javascript_message_type_;
  }
  string16 message_text() const { return message_text_; }
  string16 default_prompt_text() const { return default_prompt_text_; }
  bool display_suppress_checkbox() const { return display_suppress_checkbox_; }
  bool is_before_unload_dialog() const { return is_before_unload_dialog_; }
  bool is_reload() const { return is_reload_; }

 private:
  // Notifies the delegate with the result of the dialog.
  void NotifyDelegate(bool success, const string16& prompt_text,
                      bool suppress_js_messages);

  // The extra Runtime-only data associated with the delegate_.
  RuntimeJavaScriptDialogExtraData* extra_data_;

  // Information about the message box is held in the following variables.
  const content::JavaScriptMessageType javascript_message_type_;
  string16 message_text_;
  string16 default_prompt_text_;
  bool display_suppress_checkbox_;
  bool is_before_unload_dialog_;
  bool is_reload_;

  content::JavaScriptDialogManager::DialogClosedCallback callback_;

  // Used only for testing. Specifies alternative prompt text that should be
  // used when notifying the delegate, if |use_override_prompt_text_| is true.
  string16 override_prompt_text_;
  bool use_override_prompt_text_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptAppModalDialog);
};

#endif  // CAMEO_SRC_RUNTIME_BROWSER_UI_APP_MODAL_DIALOGS_JAVASCRIPT_APP_MODAL_DIALOG_H_
