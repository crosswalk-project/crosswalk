// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_notification_manager_win.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/desktop_notification_delegate.h"
#include "content/public/common/platform_notification_data.h"
#include "url/gurl.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_notification_win.h"

namespace xwalk {

void DismissNotification(
    XWalkNotificationWin* notification) {
  if (notification)
    notification->Dismiss();
}

XWalkNotificationManager::XWalkNotificationManager() {
  initialized_ = XWalkNotificationWin::Initialize();
}

XWalkNotificationManager::~XWalkNotificationManager() {
  for (const scoped_refptr<XWalkNotificationWin>& notification : notifications_)
    notification->Destroy();
}

void XWalkNotificationManager::ShowDesktopNotification(
    content::BrowserContext* browser_context,
    const GURL& origin,
    const content::PlatformNotificationData& notification_data,
    const content::NotificationResources& notification_resources,
    std::unique_ptr<content::DesktopNotificationDelegate> delegate,
    base::Closure* cancel_callback) {
  if (!initialized_)
    return;

  XWalkNotificationWin* notification = new XWalkNotificationWin(
      this,
      std::move(delegate));

  notifications_.insert(notification);
  notification->Show(
      notification_data.title,
      notification_data.body,
      notification_data.icon,
      notification_resources,
      notification_data.silent);
  *cancel_callback = base::Bind(&DismissNotification,
      notification);
}

void XWalkNotificationManager::RemoveNotification(
    XWalkNotificationWin* notification) {
  notification->Destroy();
  notifications_.erase(notification);
}

}  // namespace xwalk
