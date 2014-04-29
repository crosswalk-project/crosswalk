// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATIONS_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATIONS_MANAGER_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/dbus/object_manager_adaptor.h"

namespace xwalk {
namespace application {

class InstalledApplicationObject;

// Holds the D-Bus representation of the set of installed applications. This is
// the entry point for listing, installing and uninstalling applications via
// D-Bus.
//
// The exported object implements org.freedesktop.DBus.ObjectManager, and the
// interface org.crosswalkproject.Installed.Manager1 (see .cc file for
// description).
class InstalledApplicationsManager : public ApplicationService::Observer {
 public:
  InstalledApplicationsManager(scoped_refptr<dbus::Bus> bus,
                               ApplicationService* service,
                               ApplicationStorage* app_storage);
  virtual ~InstalledApplicationsManager();

 private:
  // ApplicationService::Observer implementation.
  void virtual OnApplicationInstalled(const std::string& app_id) OVERRIDE;
  void virtual OnApplicationUninstalled(const std::string& app_id) OVERRIDE;
  void virtual OnApplicationNameChanged(const std::string& app_id,
                                        const std::string& app_name) OVERRIDE;

  void AddInitialObjects();
  void AddObject(scoped_refptr<const ApplicationData> app);

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

  base::WeakPtrFactory<InstalledApplicationsManager> weak_factory_;
  ApplicationService* application_service_;
  ApplicationStorage* app_storage_;
  dbus::ObjectManagerAdaptor adaptor_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATIONS_MANAGER_H_
