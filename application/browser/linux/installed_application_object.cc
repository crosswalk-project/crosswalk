// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/installed_application_object.h"

#include "dbus/bus.h"
#include "dbus/message.h"
#include "xwalk/application/common/application.h"

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
    "org.crosswalkproject.InstalledApplication";

const char kInstalledApplicationDBusError[] =
    "org.crosswalkproject.InstalledApplication.Error";

InstalledApplicationObject::InstalledApplicationObject(
    scoped_refptr<dbus::Bus> bus, const dbus::ObjectPath& base_path,
    const Application* app)
    : app_id_(app->ID()),
      path_(dbus::ObjectPath(base_path.value() + "/" + app_id_)),
      dbus_object_(bus->GetExportedObject(path_)),
      properties_(dbus_object_, path_) {
  scoped_ptr<base::Value> app_id(base::Value::CreateStringValue(app->ID()));
  properties_.Set(kInstalledApplicationDBusInterface, "AppID", app_id.Pass());

  scoped_ptr<base::Value> name(base::Value::CreateStringValue(app->Name()));
  properties_.Set(kInstalledApplicationDBusInterface, "Name", name.Pass());
}

void InstalledApplicationObject::ExportUninstallMethod(
    dbus::ExportedObject::MethodCallCallback method_call_callback,
    dbus::ExportedObject::OnExportedCallback on_exported_callback) {
  dbus_object_->ExportMethod(
      kInstalledApplicationDBusInterface, "Uninstall",
      method_call_callback, on_exported_callback);
}

void InstalledApplicationObject::AppendAllPropertiesToWriter(
    dbus::MessageWriter* writer) {
  dbus::MessageWriter interfaces_writer(NULL);
  writer->OpenArray("{sa{sv}}", &interfaces_writer);

  dbus::MessageWriter entry_writer(NULL);
  interfaces_writer.OpenDictEntry(&entry_writer);
  entry_writer.AppendString(kInstalledApplicationDBusInterface);
  properties_.AppendPropertiesToWriter(
      kInstalledApplicationDBusInterface, &entry_writer);
  interfaces_writer.CloseContainer(&entry_writer);

  writer->CloseContainer(&interfaces_writer);
}

}  // namespace application
}  // namespace xwalk
