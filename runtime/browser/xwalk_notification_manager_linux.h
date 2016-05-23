// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_MANAGER_LINUX_H_
#define XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_MANAGER_LINUX_H_

#include <libnotify/notification.h>
#include <libnotify/notify.h>

#include <map>
#include <string>

#include "base/callback.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "base/strings/string16.h"

class GURL;

namespace content {
class BrowserContext;
class DesktopNotificationDelegate;
class RenderFrameHost;
struct PlatformNotificationData;
}  // namespace content

namespace xwalk {

class XWalkNotificationManager {
 public:
  XWalkNotificationManager();
  ~XWalkNotificationManager();

  gulong GetHandler(NotifyNotification* notification) const;

  // Show a desktop notification. If |cancel_callback| is non-null, it's set to
  // a callback which can be used to cancel the notification.
  void ShowDesktopNotification(
      content::BrowserContext* browser_context,
      const GURL& origin,
      const content::PlatformNotificationData& notification_data,
      std::unique_ptr<content::DesktopNotificationDelegate> delegate,
      base::Closure* cancel_callback);

  void NotificationDisplayed(NotifyNotification* notification);
  void NotificationClicked(NotifyNotification* notification);
  void NotificationClosed(NotifyNotification* notification);

 private:
  base::ScopedPtrHashMap<int64_t,
                         std::unique_ptr<content::DesktopNotificationDelegate>>
      notifications_map_;
  std::map<std::string, NotifyNotification*> notifications_replace_map_;
  std::map<NotifyNotification*, gulong> notifications_handler_map_;

  bool initialized_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_MANAGER_LINUX_H_
