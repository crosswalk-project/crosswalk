// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_

#include <string>

#include "base/event_types.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/tizen/app_control_info.h"
#include "xwalk/application/common/tizen/cookie_manager.h"

#include "ui/events/platform/platform_event_observer.h"
#include "ui/events/platform/platform_event_types.h"

namespace xwalk {
namespace application {

class ApplicationTizen :  // NOLINT
  public ui::PlatformEventObserver,
  public Application {
 public:
  ~ApplicationTizen() override;
  void Hide();
  void Show();
  void Suspend();
  void Resume();

  void RemoveAllCookies();
  void SetUserAgentString(const std::string& user_agent_string);

 protected:
  GURL GetStartURL(Manifest::Type type) const override;

 private:
  friend class Application;
  ApplicationTizen(scoped_refptr<ApplicationData> data,
                   XWalkBrowserContext* context);
  bool Launch() override;

  GURL GetAppControlStartURL(const AppControlInfo& app_control) const;

  base::FilePath GetSplashScreenPath() override;

  // Runtime::Observer implementation.
  void OnNewRuntimeAdded(Runtime* runtime) override;
  void OnRuntimeClosed(Runtime* runtime) override;

  void WillProcessEvent(const ui::PlatformEvent& event) override;
  void DidProcessEvent(const ui::PlatformEvent& event) override;
  bool CanBeSuspended() const;

#if defined(OS_TIZEN_MOBILE)
  NativeAppWindow* root_window_;
#endif
  scoped_ptr<CookieManager> cookie_manager_;
  bool is_suspended_;
};

inline ApplicationTizen* ToApplicationTizen(Application* app) {
  return static_cast<ApplicationTizen*>(app);
}

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
