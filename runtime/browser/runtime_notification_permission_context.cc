// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_notification_permission_context.h"

#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/strings/utf_string_conversions.h"
#include "components/prefs/pref_service.h"
#include "components/url_formatter/url_formatter.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/geolocation_provider.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "grit/xwalk_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/common/xwalk_system_locale.h"

#if !defined(OS_ANDROID)
#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_dialog_manager.h"
#endif

namespace xwalk {

void RuntimeNotificationPermissionContext::CancelNotificationPermissionRequest(
    content::WebContents* web_contents,
    const GURL& requesting_frame) {
#if !defined(OS_ANDROID)
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  XWalkPermissionDialogManager* permission_dialog_manager =
      XWalkPermissionDialogManager::FromWebContents(web_contents);
  if (!permission_dialog_manager)
    return;
  permission_dialog_manager->CancelPermissionRequest();
#endif
}

RuntimeNotificationPermissionContext::~RuntimeNotificationPermissionContext() {
}

void
RuntimeNotificationPermissionContext::RequestNotificationPermission(
    content::WebContents* web_contents,
    const GURL& requesting_frame,
    const std::string& application_name,
    base::Callback<void(bool)> result_callback) {
#if !defined(OS_ANDROID)
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  XWalkPermissionDialogManager* permission_dialog_manager =
      XWalkPermissionDialogManager::GetPermissionDialogManager(web_contents);

  PrefService* pref_service =
      user_prefs::UserPrefs::Get(XWalkBrowserContext::GetDefault());
  base::string16 text = l10n_util::GetStringFUTF16(
      IDS_NOTIFICATIONS_DIALOG_QUESTION, base::ASCIIToUTF16(application_name));

  permission_dialog_manager->RequestPermission(
      CONTENT_SETTINGS_TYPE_NOTIFICATIONS,
      requesting_frame, pref_service->GetString(kIntlAcceptLanguage), text,
      base::Bind(
          &RuntimeNotificationPermissionContext::OnPermissionRequestFinished,
          base::Unretained(this), result_callback));
#else
  result_callback.Run(false);
#endif
}

#if !defined(OS_ANDROID)
void RuntimeNotificationPermissionContext::OnPermissionRequestFinished(
    base::Callback<void(bool)> result_callback, bool success) {
  result_callback.Run(success);
}
#endif

}  // namespace xwalk
