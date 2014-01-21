// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_LINUX_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_LINUX_H_

#include "xwalk/application/browser/application_system.h"

namespace xwalk {
class DBusManager;
}

namespace xwalk {
namespace application {

class ApplicationServiceProviderLinux;

class ApplicationSystemLinux : public ApplicationSystem {
 public:
  explicit ApplicationSystemLinux(RuntimeContext* runtime_context);
  virtual ~ApplicationSystemLinux();

  DBusManager& dbus_manager();

 private:
  scoped_ptr<DBusManager> dbus_manager_;
  scoped_ptr<ApplicationServiceProviderLinux> service_provider_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationSystemLinux);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_LINUX_H_
