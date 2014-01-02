// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATIONS_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATIONS_MANAGER_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/dbus/object_manager_adaptor.h"

namespace xwalk {
namespace application {

class RunningApplicationObject;
class Application;

dbus::ObjectPath GetRunningPathForAppID(const std::string& app_id);

// Holds the D-Bus representation of the set of installed applications. This is
// the entry point for launching applications and listing currently running
// applications.
//
// The exported object implements org.freedesktop.DBus.ObjectManager, and the
// interface org.crosswalkproject.Installed.Manager1 (see .cc file for
// description).
class RunningApplicationsManager : public ApplicationService::Observer {
 public:
  RunningApplicationsManager(scoped_refptr<dbus::Bus> bus,
                             ApplicationService* service);
  ~RunningApplicationsManager();

 private:
  // org.crosswalkproject.Running.Manager1 interface.
  void OnLaunch(dbus::MethodCall* method_call,
                dbus::ExportedObject::ResponseSender response_sender);

  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success);

  void WillDestroyApplication(Application* app);

  dbus::ObjectPath AddObject(const std::string& app_id,
                             const std::string& launcher_name,
                             Application* application);

  base::WeakPtrFactory<RunningApplicationsManager> weak_factory_;
  ApplicationService* application_service_;
  dbus::ObjectManagerAdaptor adaptor_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATIONS_MANAGER_H_
