// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_TIZEN_H_

#include <string>

#include "xwalk/application/browser/application_service.h"

namespace xwalk {

namespace application {

class ApplicationStorage;

// The application service manages launch and termination of the applications.
class ApplicationServiceTizen : public ApplicationService {
 public:
  virtual ~ApplicationServiceTizen();
  // Launch an installed application using application id.
  Application* LaunchFromAppID(
      const std::string& id,
      const Application::LaunchParams& params = Application::LaunchParams());

 private:
  friend class ApplicationService;
  explicit ApplicationServiceTizen(RuntimeContext* runtime_context);
  // Note : do not export app storage from this class! We need consider
  // making ApplicationSystemTizen (owning the storage) instead.
  scoped_ptr<ApplicationStorage> application_storage_;
};

inline ApplicationServiceTizen* ToApplicationServiceTizen(
    ApplicationService* service) {
  return static_cast<ApplicationServiceTizen*>(service);
}

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_TIZEN_H_
