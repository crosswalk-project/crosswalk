// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_

#include "base/event_types.h"
#include "xwalk/application/browser/application.h"

namespace xwalk {
namespace application {

class ApplicationTizen : public Application {
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
  virtual void DidProcessEvent(const base::NativeEvent& event);
#endif
};

inline ApplicationTizen* ToApplicationTizen(Application* app) {
  return static_cast<ApplicationTizen*>(app);
}

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_TIZEN_H_
