// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_platform_notification_service.h"

#include "content/public/browser/desktop_notification_delegate.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_iterator.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/notification_resources.h"
#include "content/public/common/platform_notification_data.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#elif defined(OS_LINUX) && defined(USE_LIBNOTIFY)
#include "xwalk/runtime/browser/xwalk_notification_manager_linux.h"
#elif defined(OS_WIN)
#include "base/win/windows_version.h"
#include "xwalk/runtime/browser/xwalk_notification_manager_win.h"
#include "xwalk/runtime/browser/xwalk_content_settings.h"
#endif

namespace xwalk {

// static
XWalkPlatformNotificationService*
XWalkPlatformNotificationService::GetInstance() {
  return base::Singleton<XWalkPlatformNotificationService>::get();
}

XWalkPlatformNotificationService::XWalkPlatformNotificationService() {}

XWalkPlatformNotificationService::~XWalkPlatformNotificationService() {}

blink::mojom::PermissionStatus
XWalkPlatformNotificationService::CheckPermissionOnUIThread(
    content::BrowserContext* resource_context,
    const GURL& origin,
    int render_process_id) {
#if defined(OS_ANDROID)
  return blink::mojom::PermissionStatus::GRANTED;
#elif defined(OS_LINUX) && defined(USE_LIBNOTIFY) || defined(OS_WIN)
  return blink::mojom::PermissionStatus::GRANTED;
#elif defined(OS_WIN)
  ContentSetting setting =
      XWalkContentSettings::GetInstance()->GetPermission(
          CONTENT_SETTINGS_TYPE_NOTIFICATIONS, origin, origin);
  if (setting == CONTENT_SETTING_ALLOW)
    return blink::mojom::PermissionStatus::GRANTED;
  if (setting == CONTENT_SETTING_BLOCK)
    return blink::mojom::PermissionStatus::DENIED;

  return blink::mojom::PermissionStatus::ASK;
#else
  return blink::mojom::PermissionStatus::DENIED;
#endif
}

blink::mojom::PermissionStatus
XWalkPlatformNotificationService::CheckPermissionOnIOThread(
    content::ResourceContext* resource_context,
    const GURL& origin,
    int render_process_id) {
#if defined(OS_ANDROID)
  return blink::mojom::PermissionStatus::GRANTED;
#elif defined(OS_LINUX) && defined(USE_LIBNOTIFY)
  return blink::mojom::PermissionStatus::GRANTED;
#elif defined(OS_WIN)
  ContentSetting setting =
      XWalkContentSettings::GetInstance()->GetPermission(
          CONTENT_SETTINGS_TYPE_NOTIFICATIONS, origin, origin);
  if (setting == CONTENT_SETTING_ALLOW)
    return blink::mojom::PermissionStatus::GRANTED;
  if (setting == CONTENT_SETTING_BLOCK)
    return blink::mojom::PermissionStatus::DENIED;

  return blink::mojom::PermissionStatus::ASK;
#else
  return blink::mojom::PermissionStatus::DENIED;
#endif
}

void XWalkPlatformNotificationService::DisplayNotification(
    content::BrowserContext* browser_context,
    const GURL& origin,
    const content::PlatformNotificationData& notification_data,
    const content::NotificationResources& notification_resources,
    std::unique_ptr<content::DesktopNotificationDelegate> delegate,
    base::Closure* cancel_callback) {
#if defined(OS_ANDROID)
  std::unique_ptr<content::RenderWidgetHostIterator> widgets(
      content::RenderWidgetHost::GetRenderWidgetHosts());
  while (content::RenderWidgetHost* rwh = widgets->GetNextHost()) {
    if (!rwh->GetProcess())
      continue;
    content::RenderViewHost* rvh = content::RenderViewHost::From(rwh);
    if (!rvh)
      continue;
    content::WebContents* web_contents =
        content::WebContents::FromRenderViewHost(rvh);
    if (!web_contents)
      continue;
    XWalkContentsClientBridgeBase* bridge =
        XWalkContentsClientBridgeBase::FromWebContents(web_contents);
    bridge->ShowNotification(notification_data, notification_resources,
                             std::move(delegate), cancel_callback);
    return;
  }
#elif defined(OS_LINUX) && defined(USE_LIBNOTIFY)
  if (!notification_manager_linux_)
    notification_manager_linux_.reset(new XWalkNotificationManager());
  notification_manager_linux_->ShowDesktopNotification(
      browser_context,
      origin,
      notification_data,
      std::move(delegate),
      cancel_callback);
#elif defined(OS_WIN)
  const base::win::OSInfo* os_info = base::win::OSInfo::GetInstance();
  if (os_info->version() < base::win::VERSION_WIN8)
    return;
  if (!notification_manager_win_)
    notification_manager_win_.reset(new XWalkNotificationManager());

  notification_manager_win_->ShowDesktopNotification(
      browser_context,
      origin,
      notification_data,
      notification_resources,
      std::move(delegate),
      cancel_callback);

#endif
}

bool XWalkPlatformNotificationService::GetDisplayedPersistentNotifications(
    content::BrowserContext* browser_context,
    std::set<std::string>* displayed_notifications) {
    NOTIMPLEMENTED();
    return false;
}



}  // namespace xwalk
