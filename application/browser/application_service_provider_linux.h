// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_LINUX_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_LINUX_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace dbus {
class Bus;
}

namespace xwalk {
namespace application {

class ApplicationService;
class ApplicationStorage;
class InstalledApplicationsManager;
class RunningApplicationsManager;

// Uses a D-Bus service named "org.crosswalkproject.Runtime1" to expose
// application management and launch functionality from ApplicationService.
class ApplicationServiceProviderLinux {
 public:
  ApplicationServiceProviderLinux(ApplicationService* app_service,
                                  ApplicationStorage* app_storage,
                                  scoped_refptr<dbus::Bus> session_bus);
  virtual ~ApplicationServiceProviderLinux();

 private:
  void OnServiceNameExported(const std::string& service_name, bool success);

  // TODO(cmarcelo): Remove this once we expose real objects.
  void ExportTestObject();

  scoped_refptr<dbus::Bus> session_bus_;
  scoped_ptr<InstalledApplicationsManager> installed_apps_;
  scoped_ptr<RunningApplicationsManager> running_apps_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_LINUX_H_
