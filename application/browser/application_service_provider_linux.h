// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_LINUX_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_LINUX_H_

#include "xwalk/application/browser/application_service_provider.h"

#include "xwalk/dbus/dbus_manager.h"

namespace xwalk {
namespace application {

class InstalledApplicationsRoot;

// Uses a D-Bus service named "org.crosswalkproject" to expose application
// management and launch functionality from ApplicationService.
class ApplicationServiceProviderLinux : public ApplicationServiceProvider {
 public:
  explicit ApplicationServiceProviderLinux(ApplicationService* app_service);
  virtual ~ApplicationServiceProviderLinux();

 private:
  void OnDBusInitialized();

  // TODO(cmarcelo): Remove this once we expose real objects.
  void ExportTestObject();

  DBusManager dbus_manager_;
  scoped_ptr<InstalledApplicationsRoot> installed_apps_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_LINUX_H_
