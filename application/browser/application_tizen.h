// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_

#include "base/event_types.h"
#include "content/browser/screen_orientation/screen_orientation_provider.h"
#include "xwalk/application/browser/application.h"

#if defined(USE_OZONE)
#include "ui/events/platform/platform_event_observer.h"
#include "ui/events/platform/platform_event_types.h"
#endif

namespace xwalk {
namespace application {

class ApplicationTizen :  // NOLINT
#if defined(USE_OZONE)
  public ui::PlatformEventObserver,
#endif
  public Application, public content::ScreenOrientationProvider {
 public:
  virtual ~ApplicationTizen();
  void Hide();

 private:
  // We enforce ApplicationService ownership.
  friend class ApplicationService;
  ApplicationTizen(scoped_refptr<ApplicationData> data,
                   RuntimeContext* context,
                   Application::Observer* observer);
  virtual bool Launch(const LaunchParams& launch_params) OVERRIDE;
  virtual void InitSecurityPolicy() OVERRIDE;

#if defined(USE_OZONE)
  virtual void WillProcessEvent(const ui::PlatformEvent& event) OVERRIDE;
  virtual void DidProcessEvent(const ui::PlatformEvent& event) OVERRIDE;
#endif

  // content::ScreenOrientationProvider overrides:
  virtual void LockOrientation(
      blink::WebScreenOrientationLockType orientations) OVERRIDE;
  virtual void UnlockOrientation() OVERRIDE;
};

inline ApplicationTizen* ToApplicationTizen(Application* app) {
  return static_cast<ApplicationTizen*>(app);
}

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
