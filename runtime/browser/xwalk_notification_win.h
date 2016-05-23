// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_WIN_H_
#define XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_WIN_H_

#include <windows.h>
#include <windows.ui.notifications.h>
#include <wrl/event.h>
#include <wrl/implements.h>
#include <wrl/module.h>
#include <wrl/wrappers/corewrappers.h>
#include <string>

#include "base/callback.h"
#include "base/files/scoped_temp_dir.h"
#include "base/strings/string16.h"



namespace mswr = Microsoft::WRL;
namespace mswrw = Microsoft::WRL::Wrappers;
namespace winxml = ABI::Windows::Data::Xml;
namespace winfoundtn = ABI::Windows::Foundation;
namespace winui = ABI::Windows::UI;

using NotificationActivatedEventHandler =
    winfoundtn::ITypedEventHandler<winui::Notifications::ToastNotification*,
    IInspectable*>;
using NotificationDismissedEventHandler =
    winfoundtn::ITypedEventHandler<winui::Notifications::ToastNotification*,
    winui::Notifications::ToastDismissedEventArgs*>;
using NotificationFailedEventHandler =
    winfoundtn::ITypedEventHandler<winui::Notifications::ToastNotification*,
    winui::Notifications::ToastFailedEventArgs*>;

namespace content {
class DesktopNotificationDelegate;
struct NotificationResources;
}  // namespace content

class GURL;
class SkBitmap;

namespace xwalk {

class XWalkNotificationManager;

class XWalkNotificationWin :
    public base::RefCountedThreadSafe<XWalkNotificationWin> {
 public:
  XWalkNotificationWin(
      XWalkNotificationManager* manager,
      std::unique_ptr<content::DesktopNotificationDelegate> delegate);

  ~XWalkNotificationWin();

  static bool Initialize();

  void Destroy();
  void Show(
    const base::string16& title,
    const base::string16& body,
    const GURL& icon_url,
    const content::NotificationResources& notification_resources,
    const bool silent);

  void Dismiss();

 private:
  friend class NotificationEventHandlerWin;
  void NotificationClosed();
  void NotificationClicked();
  void SaveIconToFilesystemAndProceed(
      const base::string16& title,
      const base::string16& body,
      const GURL& icon_url,
      const SkBitmap& icon,
      const bool silent);
  bool SetImageSrc(const std::wstring& imagePath,
                   winxml::Dom::IXmlDocument *toastXml);
  bool SetXmlText(winxml::Dom::IXmlDocument* doc, const std::wstring& text);
  bool SetXmlText(winxml::Dom::IXmlDocument* doc,
                  const std::wstring& title,
                  const std::wstring& body);
  bool SetXmlAudioSilent(winxml::Dom::IXmlDocument* doc);
  bool GetTextNodeList(
      winxml::Dom::IXmlDocument* doc,
      winxml::Dom::IXmlNodeList** nodeList,
      uint32_t reqLength);
  bool AppendTextToXml(
      winxml::Dom::IXmlDocument* doc,
      winxml::Dom::IXmlNode* node,
      const std::wstring& text);
  bool CreateToastXml(
      const std::wstring& title,
      const std::wstring& body,
      const std::wstring& icon_path,
      const bool silent,
      winxml::Dom::IXmlDocument** input_xml);

  void ShowNotification(
      const base::string16& title,
      const base::string16& body,
      const base::string16& icon_path,
      const bool silent);
  void InitTempDir();
  void CleanupTempDir();

  static mswr::ComPtr<winui::Notifications::IToastNotificationManagerStatics>
        toast_manager_;
  static mswr::ComPtr<winui::Notifications::IToastNotifier> toast_notifier_;

  base::ScopedTempDir temp_dir_;
  XWalkNotificationManager* manager_;
  std::unique_ptr<content::DesktopNotificationDelegate> delegate_;
  EventRegistrationToken activated_token_;
  EventRegistrationToken dismissed_token_;
  EventRegistrationToken failed_token_;

  mswr::ComPtr<NotificationEventHandlerWin> event_handler_;
  mswr::ComPtr<winui::Notifications::IToastNotification> toast_notification_;

  DISALLOW_COPY_AND_ASSIGN(XWalkNotificationWin);
};

class NotificationEventHandlerWin :
    public mswr::RuntimeClass<mswr::RuntimeClassFlags<mswr::ClassicCom>,
                              NotificationActivatedEventHandler,
                              NotificationDismissedEventHandler,
                              NotificationFailedEventHandler> {
 public:
  NotificationEventHandlerWin(
      XWalkNotificationWin* notification);
  ~NotificationEventHandlerWin();

  IFACEMETHODIMP Invoke(winui::Notifications::IToastNotification* sender,
      IInspectable* args);
  IFACEMETHODIMP Invoke(winui::Notifications::IToastNotification* sender,
      winui::Notifications::IToastDismissedEventArgs* e);
  IFACEMETHODIMP Invoke(winui::Notifications::IToastNotification* sender,
      winui::Notifications::IToastFailedEventArgs* e);

 private:
  XWalkNotificationWin* notification_;

  DISALLOW_COPY_AND_ASSIGN(NotificationEventHandlerWin);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_NOTIFICATION_WIN_H_
