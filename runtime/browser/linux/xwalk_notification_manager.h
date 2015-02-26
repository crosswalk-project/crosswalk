// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_LINUX_XWALK_NOTIFICATION_MANAGER_H_
#define XWALK_RUNTIME_BROWSER_LINUX_XWALK_NOTIFICATION_MANAGER_H_

#include <libnotify/notification.h>
#include <libnotify/notify.h>

#include <map>

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

  // Show a desktop notification. If |cancel_callback| is non-null, it's set to
  // a callback which can be used to cancel the notification.
  void ShowDesktopNotification(
      content::BrowserContext* browser_context,
      const GURL& origin,
      const content::PlatformNotificationData& notification_data,
      scoped_ptr<content::DesktopNotificationDelegate> delegate,
      int render_process_id,
      base::Closure* cancel_callback);

  void NotificationDisplayed(NotifyNotification* notification);
  void NotificationClicked(NotifyNotification* notification);
  void NotificationClosed(NotifyNotification* notification, bool by_user);

  gulong GetClosedHandler(NotifyNotification* notification) const {
    return notifications_handler_map_.find(notification) !=
        notifications_handler_map_.end() ?
        notifications_handler_map_.find(notification)->second : 0;
  }

 private:
  base::ScopedPtrHashMap<int64, content::DesktopNotificationDelegate>
      notifications_map_;
  std::map<base::string16, NotifyNotification*> notifications_replace_map_;
  std::map<NotifyNotification*, gulong> notifications_handler_map_;

  bool initialized_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_LINUX_XWALK_NOTIFICATION_MANAGER_H_
