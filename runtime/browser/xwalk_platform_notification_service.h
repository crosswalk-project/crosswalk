// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PLATFORM_NOTIFICATION_SERVICE_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PLATFORM_NOTIFICATION_SERVICE_H_

#include <string>

#include "base/memory/singleton.h"
#include "content/public/browser/platform_notification_service.h"

namespace xwalk {

// The platform notification service is the profile-agnostic entry point
// through which Web Notifications can be controlled. Heavily based on
// src/chrome/browser/notifications/platform_notification_service_impl.h.
class XWalkPlatformNotificationService
    : public content::PlatformNotificationService {
 public:
  // Returns the active instance of the service in the browser process. Safe to
  // be called from any thread.
  static XWalkPlatformNotificationService* GetInstance();

  // content::PlatformNotificationService implementation.
  blink::WebNotificationPermission CheckPermission(
      content::ResourceContext* resource_context,
      const GURL& origin,
      int render_process_id) override;
  void DisplayNotification(
      content::BrowserContext* browser_context,
      const GURL& origin,
      const SkBitmap& icon,
      const content::PlatformNotificationData& notification_data,
      scoped_ptr<content::DesktopNotificationDelegate> delegate,
      int render_process_id,
      base::Closure* cancel_callback) override;
  void DisplayPersistentNotification(
      content::BrowserContext* browser_context,
      int64 service_worker_registration_id,
      const GURL& origin,
      const SkBitmap& icon,
      const content::PlatformNotificationData& notification_data,
      int render_process_id) override {}
  void ClosePersistentNotification(
      content::BrowserContext* browser_context,
      const std::string& persistent_notification_id) override {}

 private:
  friend struct DefaultSingletonTraits<XWalkPlatformNotificationService>;

  XWalkPlatformNotificationService();
  ~XWalkPlatformNotificationService() override;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PLATFORM_NOTIFICATION_SERVICE_H_
