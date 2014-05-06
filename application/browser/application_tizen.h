// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_

#include "xwalk/application/browser/application.h"

#if defined(USE_OZONE)
#include "base/message_loop/message_pump_observer.h"
#endif

namespace xwalk {
namespace application {

class ApplicationTizen : //  NOLINT
#if defined(USE_OZONE)
  public base::MessagePumpObserver,
#endif
  public Application {
 public:
  virtual ~ApplicationTizen();
  void Hide();

 private:
  // We enforce ApplicationService ownership.
  friend class ApplicationService;
  ApplicationTizen(scoped_refptr<ApplicationData> data,
                   RuntimeContext* context,
                   Application::Observer* observer);

  virtual void InitSecurityPolicy() OVERRIDE;

#if defined(USE_OZONE)
  virtual base::EventStatus WillProcessEvent(
      const base::NativeEvent& event) OVERRIDE;
  virtual void DidProcessEvent(const base::NativeEvent& event) OVERRIDE;
#endif
};

inline ApplicationTizen* ToApplicationTizen(Application* app) {
  return static_cast<ApplicationTizen*>(app);
}

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
