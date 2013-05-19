// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/ui/app_modal_dialogs/javascript_dialog_manager.h"

#include <map>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/i18n/rtl.h"
#include "base/memory/singleton.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/runtime/browser/runtime.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/app_modal_dialog.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/app_modal_dialog_queue.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/javascript_app_modal_dialog.h"
#include "cameo/src/runtime/browser/ui/app_modal_dialogs/native_app_modal_dialog.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "content/public/common/content_client.h"
#include "content/public/common/javascript_message_type.h"
#include "grit/cameo_resources.h"
#include "net/base/net_util.h"
#include "ui/base/l10n/l10n_util.h"

using content::JavaScriptDialogManager;
using content::WebContents;

namespace cameo {

class RuntimeJavaScriptDialogManager : public JavaScriptDialogManager,
                                       public content::NotificationObserver {
 public:
  static RuntimeJavaScriptDialogManager* GetInstance();

  explicit RuntimeJavaScriptDialogManager(
      cameo::Runtime* runtime);
  virtual ~RuntimeJavaScriptDialogManager();

  virtual void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType message_type,
      const string16& message_text,
      const string16& default_prompt_text,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) OVERRIDE;

  virtual void RunBeforeUnloadDialog(
      content::WebContents* web_contents,
      const string16& message_text,
      bool is_reload,
      const DialogClosedCallback& callback) OVERRIDE;

  virtual bool HandleJavaScriptDialog(
      content::WebContents* web_contents,
      bool accept,
      const string16* prompt_override) OVERRIDE;

  virtual void ResetJavaScriptState(
      content::WebContents* web_contents) OVERRIDE;

 private:
  RuntimeJavaScriptDialogManager();

  friend struct DefaultSingletonTraits<RuntimeJavaScriptDialogManager>;

  // Overridden from content::NotificationObserver:
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  string16 GetTitle(const GURL& origin_url,
                    const std::string& accept_lang,
                    bool is_alert);

  void CancelPendingDialogs(content::WebContents* web_contents);

  // Wrapper around a DialogClosedCallback so that we can intercept it before
  // passing it onto the original callback.
  void OnDialogClosed(DialogClosedCallback callback,
                      bool success,
                      const string16& user_input);

  // Mapping between the WebContents and their extra data. The key
  // is a void* because the pointer is just a cookie and is never dereferenced.
  typedef std::map<void*, RuntimeJavaScriptDialogExtraData>
      JavaScriptDialogExtraDataMap;
  JavaScriptDialogExtraDataMap javascript_dialog_extra_data_;

  // Runtime which owns the RuntimeJavaScriptDialogManager instance.
  // It's used to get a app name from a URL.
  // If it's not owned by any app, it should be NULL.
  cameo::Runtime* runtime_;

  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(RuntimeJavaScriptDialogManager);
};

////////////////////////////////////////////////////////////////////////////////
// RuntimeJavaScriptDialogManager, public:

RuntimeJavaScriptDialogManager::RuntimeJavaScriptDialogManager()
    : runtime_(NULL) {
}

RuntimeJavaScriptDialogManager::~RuntimeJavaScriptDialogManager() {
  runtime_ = NULL;
}

RuntimeJavaScriptDialogManager::RuntimeJavaScriptDialogManager(
    cameo::Runtime* runtime)
    : runtime_(runtime) {
}

// static
RuntimeJavaScriptDialogManager* RuntimeJavaScriptDialogManager::GetInstance() {
  return Singleton<RuntimeJavaScriptDialogManager>::get();
}

void RuntimeJavaScriptDialogManager::RunJavaScriptDialog(
    content::WebContents* web_contents,
    const GURL& origin_url,
    const std::string& accept_lang,
    content::JavaScriptMessageType message_type,
    const string16& message_text,
    const string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message)  {
  *did_suppress_message = false;

  RuntimeJavaScriptDialogExtraData* extra_data =
      &javascript_dialog_extra_data_[web_contents];

  if (extra_data->suppress_javascript_messages_) {
    *did_suppress_message = true;
    return;
  }

  bool display_suppress_checkbox = false;

  bool is_alert = message_type == content::JAVASCRIPT_MESSAGE_TYPE_ALERT;
  string16 dialog_title = GetTitle(origin_url, accept_lang, is_alert);

  AppModalDialogQueue::GetInstance()->AddDialog(new JavaScriptAppModalDialog(
      web_contents,
      extra_data,
      dialog_title,
      message_type,
      message_text,
      default_prompt_text,
      display_suppress_checkbox,
      false,  // is_before_unload_dialog
      false,  // is_reload
      base::Bind(&RuntimeJavaScriptDialogManager::OnDialogClosed,
                 base::Unretained(this), callback)));
}

void RuntimeJavaScriptDialogManager::RunBeforeUnloadDialog(
    content::WebContents* web_contents,
    const string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) {
  RuntimeJavaScriptDialogExtraData* extra_data =
      &javascript_dialog_extra_data_[web_contents];

  const string16 title = l10n_util::GetStringUTF16(is_reload ?
      IDS_BEFORERELOAD_MESSAGEBOX_TITLE : IDS_BEFOREUNLOAD_MESSAGEBOX_TITLE);
  const string16 footer = l10n_util::GetStringUTF16(is_reload ?
      IDS_BEFORERELOAD_MESSAGEBOX_FOOTER : IDS_BEFOREUNLOAD_MESSAGEBOX_FOOTER);

  string16 full_message = message_text + ASCIIToUTF16("\n\n") + footer;

  AppModalDialogQueue::GetInstance()->AddDialog(new JavaScriptAppModalDialog(
      web_contents,
      extra_data,
      title,
      content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM,
      full_message,
      string16(),  // default_prompt_text
      false,       // display_suppress_checkbox
      true,        // is_before_unload_dialog
      is_reload,
      base::Bind(&RuntimeJavaScriptDialogManager::OnDialogClosed,
                 base::Unretained(this), callback)));
}

bool RuntimeJavaScriptDialogManager::HandleJavaScriptDialog(
    content::WebContents* web_contents,
    bool accept,
    const string16* prompt_override) {
  AppModalDialogQueue* dialog_queue = AppModalDialogQueue::GetInstance();
  if (!dialog_queue->HasActiveDialog() ||
      !dialog_queue->active_dialog()->IsJavaScriptModalDialog() ||
      dialog_queue->active_dialog()->web_contents() != web_contents) {
    return false;
  }
  JavaScriptAppModalDialog* dialog = static_cast<JavaScriptAppModalDialog*>(
      dialog_queue->active_dialog());
  if (accept) {
    if (prompt_override)
      dialog->SetOverridePromptText(*prompt_override);
    dialog->native_dialog()->AcceptAppModalDialog();
  } else {
    dialog->native_dialog()->CancelAppModalDialog();
  }
  return true;
}

void RuntimeJavaScriptDialogManager::ResetJavaScriptState(
    content::WebContents* web_contents) {
  CancelPendingDialogs(web_contents);
  javascript_dialog_extra_data_.erase(web_contents);
}

void RuntimeJavaScriptDialogManager::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  runtime_ = NULL;
}

string16 RuntimeJavaScriptDialogManager::GetTitle(const GURL& origin_url,
                                                 const std::string& accept_lang,
                                                 bool is_alert) {
  // If the URL hasn't any host, return the default string.
  if (!origin_url.has_host()) {
      return l10n_util::GetStringUTF16(
          is_alert ? IDS_JAVASCRIPT_ALERT_DEFAULT_TITLE
                   : IDS_JAVASCRIPT_MESSAGEBOX_DEFAULT_TITLE);
  }

  // TODO(junmin): If there exists app name, return it.

  // Otherwise, return the formatted URL.
  // In this case, force URL to have LTR directionality.
  string16 url_string = net::FormatUrl(origin_url, accept_lang);
  return l10n_util::GetStringFUTF16(
      is_alert ? IDS_JAVASCRIPT_ALERT_TITLE
      : IDS_JAVASCRIPT_MESSAGEBOX_TITLE,
      base::i18n::GetDisplayStringInLTRDirectionality(url_string));
}

void RuntimeJavaScriptDialogManager::CancelPendingDialogs(
    content::WebContents* web_contents) {
  AppModalDialogQueue* queue = AppModalDialogQueue::GetInstance();
  AppModalDialog* active_dialog = queue->active_dialog();
  if (active_dialog && active_dialog->web_contents() == web_contents)
    active_dialog->Invalidate();
  for (AppModalDialogQueue::iterator i = queue->begin();
       i != queue->end(); ++i) {
    if ((*i)->web_contents() == web_contents)
      (*i)->Invalidate();
  }
}

void RuntimeJavaScriptDialogManager::OnDialogClosed(
    DialogClosedCallback callback,
    bool success,
    const string16& user_input) {
  callback.Run(success, user_input);
}

content::JavaScriptDialogManager* GetJavaScriptDialogManagerInstance() {
  return RuntimeJavaScriptDialogManager::GetInstance();
}

content::JavaScriptDialogManager* CreateJavaScriptDialogManagerInstance(
    cameo::Runtime* runtime) {
  return new RuntimeJavaScriptDialogManager(runtime);
}

}  // namespace cameo
