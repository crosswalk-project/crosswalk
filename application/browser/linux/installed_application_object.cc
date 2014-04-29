// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/installed_application_object.h"

#include "dbus/bus.h"
#include "dbus/message.h"
#include "xwalk/application/common/application_data.h"

namespace xwalk {
namespace application {

// D-Bus Interface implemented by objects that represent installed
// applications.
//
// Methods:
//
//   Uninstall()
//     Will uninstall the application. This object will be unregistered from
//     D-Bus.
//
// Properties:
//
//   readonly string AppID
//   readonly string Name
const char kInstalledApplicationDBusInterface[] =
    "org.crosswalkproject.Installed.Application1";

const char kInstalledApplicationDBusError[] =
    "org.crosswalkproject.Installed.Application.Error";

InstalledApplicationObject::InstalledApplicationObject(
    scoped_refptr<dbus::Bus> bus, const dbus::ObjectPath& path,
    const ApplicationData* app)
    : dbus::ManagedObject(bus, path),
      app_id_(app->ID()) {
  scoped_ptr<base::Value> app_id(base::Value::CreateStringValue(app->ID()));
  properties()->Set(kInstalledApplicationDBusInterface, "AppID", app_id.Pass());

  scoped_ptr<base::Value> name(base::Value::CreateStringValue(app->Name()));
  properties()->Set(kInstalledApplicationDBusInterface, "Name", name.Pass());
}

void InstalledApplicationObject::OnApplicationNameChanged(
    const std::string& app_name) {
  scoped_ptr<base::Value> name(base::Value::CreateStringValue(app_name));
  properties()->Set(kInstalledApplicationDBusInterface, "Name", name.Pass());
}

void InstalledApplicationObject::ExportUninstallMethod(
    dbus::ExportedObject::MethodCallCallback method_call_callback,
    dbus::ExportedObject::OnExportedCallback on_exported_callback) {
  dbus_object()->ExportMethod(
      kInstalledApplicationDBusInterface, "Uninstall",
      method_call_callback, on_exported_callback);
}

}  // namespace application
}  // namespace xwalk
