// Copyright 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_dialog_manager.h"

#include "base/bind.h"
#include "base/i18n/rtl.h"
#include "base/strings/utf_string_conversions.h"
#include "components/app_modal/app_modal_dialog.h"
#include "components/app_modal/app_modal_dialog_queue.h"
#include "components/app_modal/native_app_modal_dialog.h"
#include "grit/components_strings.h"
#include "xwalk/runtime/browser/ui/desktop/xwalk_permission_modal_dialog.h"
#include "xwalk/runtime/browser/xwalk_content_settings.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(xwalk::XWalkPermissionDialogManager);

namespace xwalk {

XWalkPermissionDialogManager::XWalkPermissionDialogManager(
    content::WebContents* web_contents) :
    content::WebContentsObserver(web_contents),
    web_contents_(web_contents) {
  DCHECK(web_contents_);
}

XWalkPermissionDialogManager::~XWalkPermissionDialogManager() {
  CancelPermissionRequest();
}

XWalkPermissionDialogManager*
    XWalkPermissionDialogManager::GetPermissionDialogManager(
        content::WebContents* web_contents) {
  XWalkPermissionDialogManager* permission_dialog_manager =
      XWalkPermissionDialogManager::FromWebContents(web_contents);
  if (!permission_dialog_manager) {
    XWalkPermissionDialogManager::CreateForWebContents(web_contents);
    permission_dialog_manager =
        XWalkPermissionDialogManager::FromWebContents(web_contents);
  }
  return permission_dialog_manager;
}

void XWalkPermissionDialogManager::WebContentsDestroyed() {
  CancelPermissionRequest();
  web_contents_->RemoveUserData(UserDataKey());
}

void XWalkPermissionDialogManager::RequestPermission(
    ContentSettingsType type,
    const GURL& origin_url,
    const std::string& accept_lang,
    const base::string16& message_text,
    const base::Callback<void(bool)>& callback) {
  if (callback.is_null()) {
    return;
  }

  ContentSetting content_setting =
      XWalkContentSettings::GetInstance()->GetPermission(
          type,
          origin_url,
          web_contents_->GetLastCommittedURL().GetOrigin());

  switch (content_setting) {
  case CONTENT_SETTING_ALLOW :
    callback.Run(true);
    break;
  case CONTENT_SETTING_BLOCK:
    callback.Run(false);
    break;
  default: {
    app_modal::AppModalDialogQueue::GetInstance()->AddDialog(
        new XWalkPermissionModalDialog(
            web_contents_,
            message_text,
            base::Bind(&XWalkPermissionDialogManager::OnPermissionDialogClosed,
            base::Unretained(this), type, origin_url, callback)));
  }
  }
}

void XWalkPermissionDialogManager::CancelPermissionRequest() {
  app_modal::AppModalDialogQueue* queue =
      app_modal::AppModalDialogQueue::GetInstance();
  app_modal::AppModalDialog* active_dialog = queue->active_dialog();
  for (auto it = queue->begin(); it != queue->end(); ++it) {
    // Invalidating the active dialog might trigger showing a not-yet
    // invalidated dialog, so invalidate the active dialog last.
    if ((*it) == active_dialog)
      continue;
    if ((*it)->web_contents() == web_contents_)
      (*it)->Invalidate();
  }
  if (active_dialog && active_dialog->web_contents() == web_contents_)
    active_dialog->Invalidate();
}

void XWalkPermissionDialogManager::OnPermissionDialogClosed(
    ContentSettingsType type,
    const GURL& origin_url,
    const base::Callback<void(bool)>& callback,
    bool success) {
  ContentSetting content_setting =
      success ? CONTENT_SETTING_ALLOW : CONTENT_SETTING_BLOCK;
  XWalkContentSettings::GetInstance()->SetPermission(
      type,
      origin_url,
      web_contents_->GetLastCommittedURL().GetOrigin(),
      content_setting);
  if (!callback.is_null())
    callback.Run(success);
}

}  // namespace xwalk
