// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_MANAGER_WIN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_MANAGER_WIN_H_

#include <set>
#include <string>

#include "base/callback.h"
#include "base/memory/weak_ptr.h"

class GURL;
class SkBitmap;

namespace content {
class BrowserContext;
class DesktopNotificationDelegate;
struct NotificationResources;
struct PlatformNotificationData;
}  // namespace content

namespace xwalk {

class XWalkNotificationWin;

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
      const content::NotificationResources& notification_resources,
      std::unique_ptr<content::DesktopNotificationDelegate> delegate,
      base::Closure* cancel_callback);
  void RemoveNotification(XWalkNotificationWin* notification);
 private:
  bool initialized_;
  std::set<scoped_refptr<XWalkNotificationWin>> notifications_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_MANAGER_WIN_H_
