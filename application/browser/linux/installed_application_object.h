// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATION_OBJECT_H_
#define XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATION_OBJECT_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "xwalk/dbus/object_manager_adaptor.h"

namespace xwalk {
namespace application {

class ApplicationData;

extern const char kInstalledApplicationDBusInterface[];
extern const char kInstalledApplicationDBusError[];

// Holds the D-Bus representation of an installed application. These objects are
// owned by InstalledApplicationsManager and the corresponding objects in D-Bus
// are child of '/installed/' object path.
class InstalledApplicationObject : public dbus::ManagedObject {
 public:
  InstalledApplicationObject(
      scoped_refptr<dbus::Bus> bus, const dbus::ObjectPath& path,
      const ApplicationData* app);

  void OnApplicationNameChanged(const std::string& app_name);
  // Set the callback used when the Uninstall() method is called in an
  // ApplicationObject.
  void ExportUninstallMethod(
      dbus::ExportedObject::MethodCallCallback method_call_callback,
      dbus::ExportedObject::OnExportedCallback on_exported_callback);

  std::string app_id() const { return app_id_; }

 private:
  std::string app_id_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_LINUX_INSTALLED_APPLICATION_OBJECT_H_
