// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_notification_manager_linux.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/desktop_notification_delegate.h"
#include "content/public/common/platform_notification_data.h"
#include "url/gurl.h"

namespace {

using content::BrowserThread;
using xwalk::XWalkNotificationManager;

// Defined by 'org.freedesktop.Notifications'.
const int g_closed_by_user = 2;

void NotificationClosedCallback(NotifyNotification* notification,
                                gpointer user_data) {
  XWalkNotificationManager* service =
      static_cast<XWalkNotificationManager*>(user_data);

  gint reason = notify_notification_get_closed_reason(notification);

  bool by_user = (reason == g_closed_by_user);
  if (by_user)
    service->NotificationClicked(notification);
  service->NotificationClosed(notification);
  g_signal_handler_disconnect(notification,
                              service->GetHandler(notification));
}

void CancelDesktopNotificationCallback(
    XWalkNotificationManager* service,
    NotifyNotification* notification) {
  g_signal_handler_disconnect(notification,
                              service->GetHandler(notification));
  notify_notification_close(notification, nullptr);
}

}  // namespace

namespace xwalk {

XWalkNotificationManager::XWalkNotificationManager() {
  initialized_ = notify_init("xwalk");
}

XWalkNotificationManager::~XWalkNotificationManager() {
  if (initialized_)
    notify_uninit();
}

gulong XWalkNotificationManager::GetHandler(
    NotifyNotification* notification) const {
  auto i = notifications_handler_map_.find(notification);
  return i != notifications_handler_map_.end() ? i->second : 0;
}

void XWalkNotificationManager::ShowDesktopNotification(
    content::BrowserContext* browser_context,
    const GURL& origin,
    const content::PlatformNotificationData& notification_data,
    std::unique_ptr<content::DesktopNotificationDelegate> delegate,
    base::Closure* cancel_callback) {
  if (!initialized_)
    return;

  NotifyNotification* notification = nullptr;

  if (!notification_data.tag.empty() &&
      notifications_replace_map_.find(notification_data.tag) !=
          notifications_replace_map_.end()) {
    notification = notifications_replace_map_[notification_data.tag];
    notify_notification_update(notification,
        base::UTF16ToUTF8(notification_data.title).c_str(),
        base::UTF16ToUTF8(notification_data.body).c_str(),
        nullptr);
  } else {
    notification = notify_notification_new(
        base::UTF16ToUTF8(notification_data.title).c_str(),
        base::UTF16ToUTF8(notification_data.body).c_str(),
        nullptr);

    notifications_map_.set(reinterpret_cast<int64_t>(notification),
                           std::move(delegate));
    if (!notification_data.tag.empty()) {
      notifications_replace_map_[notification_data.tag] = notification;
    }

    notifications_handler_map_[notification] =
        g_signal_connect(G_OBJECT(notification),
                         "closed",
                         G_CALLBACK(NotificationClosedCallback),
                         this);
    if (cancel_callback)
      *cancel_callback = base::Bind(&CancelDesktopNotificationCallback,
                                    this,
                                    notification);
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&XWalkNotificationManager::NotificationDisplayed,
        base::Unretained(this),
        notification));
  }

  notify_notification_show(notification, nullptr);
}

void XWalkNotificationManager::NotificationDisplayed(
    NotifyNotification* notification) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  content::DesktopNotificationDelegate* notification_delegate =
      notifications_map_.get(reinterpret_cast<int64_t>(notification));
  if (notification_delegate)
    notification_delegate->NotificationDisplayed();
}

void XWalkNotificationManager::NotificationClicked(
    NotifyNotification* notification) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  content::DesktopNotificationDelegate* notification_delegate =
      notifications_map_.get(reinterpret_cast<int64_t>(notification));
  if (notification_delegate) {
    notification_delegate->NotificationClick();
  }
}

void XWalkNotificationManager::NotificationClosed(
    NotifyNotification* notification) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  std::unique_ptr<content::DesktopNotificationDelegate> notification_delegate =
    notifications_map_.take_and_erase(reinterpret_cast<int64_t>(notification));
  if (notification_delegate) {
    notification_delegate->NotificationClosed();
  }
}

}  // namespace xwalk
