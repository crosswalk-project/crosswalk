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
#include "content/public/common/platform_notification_data.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#endif

namespace xwalk {

// static
XWalkPlatformNotificationService*
XWalkPlatformNotificationService::GetInstance() {
  return Singleton<XWalkPlatformNotificationService>::get();
}

XWalkPlatformNotificationService::XWalkPlatformNotificationService() {}

XWalkPlatformNotificationService::~XWalkPlatformNotificationService() {}

blink::WebNotificationPermission
XWalkPlatformNotificationService::CheckPermission(
    content::ResourceContext* resource_context,
    const GURL& origin,
    int render_process_id) {
#if defined(OS_ANDROID)
  return blink::WebNotificationPermissionAllowed;
#else
  return blink::WebNotificationPermissionDenied;
#endif
}

void XWalkPlatformNotificationService::DisplayNotification(
    content::BrowserContext* browser_context,
    const GURL& origin,
    const SkBitmap& icon,
    const content::PlatformNotificationData& notification_data,
    scoped_ptr<content::DesktopNotificationDelegate> delegate,
    int render_process_id,
    base::Closure* cancel_callback) {
#if defined(OS_ANDROID)
  scoped_ptr<content::RenderWidgetHostIterator> widgets(
      content::RenderWidgetHost::GetRenderWidgetHosts());
  while (content::RenderWidgetHost* rwh = widgets->GetNextHost()) {
    if (!rwh->GetProcess() || rwh->GetProcess()->GetID() != render_process_id)
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
    bridge->ShowNotification(notification_data, icon, delegate.Pass(),
                             cancel_callback);
    return;
  }
#endif
}

}  // namespace xwalk
