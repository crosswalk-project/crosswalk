// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATIONS_ROOT_H_
#define XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATIONS_ROOT_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "dbus/exported_object.h"
#include "xwalk/application/browser/application_service.h"

namespace xwalk {
namespace application {

// Holds the D-Bus representation of the set of installed applications. This is
// the entry point for launching applications and listing currently running
// applications.
class RunningApplicationsManager {
 public:
  RunningApplicationsManager(scoped_refptr<dbus::Bus> bus,
                             ApplicationService* service);
  ~RunningApplicationsManager();

 private:
  void OnLaunch(dbus::MethodCall* method_call,
                dbus::ExportedObject::ResponseSender response_sender);
  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success);

  base::WeakPtrFactory<RunningApplicationsManager> weak_factory_;

  ApplicationService* application_service_;
  dbus::Bus* bus_;
  dbus::ExportedObject* root_object_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATIONS_ROOT_H_
