// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATIONS_ROOT_H_
#define XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATIONS_ROOT_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "dbus/exported_object.h"
#include "xwalk/application/browser/application_service.h"

namespace xwalk {
namespace application {

class InstalledApplicationObject;

// Holds the D-Bus representation of the set of installed applications. This is
// the entry point for listing, installing and uninstalling applications via
// D-Bus.
//
// The exported object implements org.freedesktop.DBus.ObjectManager, and the
// interface org.crosswalkproject.InstalledApplicationsRoot (see .cc file for
// description).
class InstalledApplicationsRoot : public ApplicationService::Observer {
 public:
  InstalledApplicationsRoot(scoped_refptr<dbus::Bus> bus,
                            ApplicationService* service);
  ~InstalledApplicationsRoot();

 private:
  // ApplicationService::Observer implementation.
  void OnApplicationInstalled(const std::string& app_id);
  void OnApplicationUninstalled(const std::string& app_id);

  void CreateInitialObjects();

  InstalledApplicationObject* CreateObject(
      scoped_refptr<const Application> app);

  void OnGetManagedObjects(
      dbus::MethodCall* method_call,
      dbus::ExportedObject::ResponseSender response_sender);
  void OnInstall(
      dbus::MethodCall* method_call,
      dbus::ExportedObject::ResponseSender response_sender);
  void OnUninstall(
      InstalledApplicationObject* installed_app_object,
      dbus::MethodCall* method_call,
      dbus::ExportedObject::ResponseSender response_sender);
  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success);

  base::WeakPtrFactory<InstalledApplicationsRoot> weak_factory_;

  ApplicationService* application_service_;
  dbus::Bus* bus_;
  dbus::ExportedObject* root_object_;

  ScopedVector<InstalledApplicationObject> installed_apps_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATIONS_ROOT_H_
