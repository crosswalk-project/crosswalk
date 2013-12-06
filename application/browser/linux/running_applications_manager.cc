// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/running_applications_manager.h"

#include <string>
#include "base/bind.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "xwalk/runtime/browser/runtime_registry.h"

namespace {

// D-Bus Interface implemented by the manager object of running applications.
//
// Methods:
//
//   Launch(string app_id) -> ObjectPath
//     Launches the application with 'app_id'.
const char kRunningManagerDBusInterface[] =
    "org.crosswalkproject.Running.Manager";

const char kRunningManagerDBusError[] =
    "org.crosswalkproject.Running.Manager.Error";

const dbus::ObjectPath kRunningManagerDBusPath("/running");

// D-Bus Interface implemented by objects that represent running
// applications.
//
// Methods:
//
//   Terminate()
//     Will terminate the running application. This object will be unregistered
//     from D-Bus.
//
// Properties:
//
//   readonly string AppID
const char kRunningApplicationDBusInterface[] =
    "org.crosswalkproject.Running.Application";

const char kRunningApplicationDBusError[] =
    "org.crosswalkproject.Running.Application.Error";

dbus::ObjectPath GetRunningPathForAppID(const std::string& app_id) {
  return dbus::ObjectPath(kRunningManagerDBusPath.value() + "/" + app_id);
}

}  // namespace

namespace xwalk {
namespace application {

RunningApplicationsManager::RunningApplicationsManager(
    scoped_refptr<dbus::Bus> bus, ApplicationService* service)
    : weak_factory_(this),
      application_service_(service),
      adaptor_(bus, kRunningManagerDBusPath) {
  adaptor_.manager_object()->ExportMethod(
      kRunningManagerDBusInterface, "Launch",
      base::Bind(&RunningApplicationsManager::OnLaunch,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&RunningApplicationsManager::OnExported,
                 weak_factory_.GetWeakPtr()));
}

RunningApplicationsManager::~RunningApplicationsManager() {}

namespace {

scoped_ptr<dbus::Response> CreateError(dbus::MethodCall* method_call,
                                       const std::string& message) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kRunningManagerDBusError, message);
    return error_response.PassAs<dbus::Response>();
}

}  // namespace

void RunningApplicationsManager::OnLaunch(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {

  dbus::MessageReader reader(method_call);
  std::string app_id;
  if (!reader.PopString(&app_id)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error parsing message. Missing app_id argument.");
    response_sender.Run(response.Pass());
    return;
  }

  if (!application_service_->Launch(app_id)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error launching application with id " + app_id);
    response_sender.Run(response.Pass());
    return;
  }

  // FIXME(cmarcelo): ApplicationService should tell us when new applications
  // appear and we create new managed objects in D-Bus based on that. See
  // InstalledApplicationManager for an example. We also have to store the
  // response_sender associated with that app_id.
  AddObject(app_id);

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendObjectPath(GetRunningPathForAppID(app_id));
  response_sender.Run(response.Pass());
}

void RunningApplicationsManager::OnTerminate(
    dbus::ManagedObject* object, dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {

  // FIXME(cmarcelo): While there's still no notion of Running Application yet,
  // we'll simply close all the windows of the current one.
  RuntimeRegistry::Get()->CloseAll();

  adaptor_.RemoveManagedObject(object->path());

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}

void RunningApplicationsManager::OnExported(
    const std::string& interface_name,
    const std::string& method_name,
    bool success) {
  if (!success) {
    LOG(WARNING) << "Error exporting method '" << interface_name
                 << "." << method_name << "' in '"
                 << kRunningManagerDBusPath.value() << "'.";
  }
}

void RunningApplicationsManager::AddObject(const std::string& app_id) {
  scoped_ptr<dbus::ManagedObject> object(
      new dbus::ManagedObject(adaptor_.bus(), GetRunningPathForAppID(app_id)));
  object->dbus_object()->ExportMethod(
      kRunningApplicationDBusInterface, "Terminate",
      base::Bind(&RunningApplicationsManager::OnTerminate,
                 weak_factory_.GetWeakPtr(),
                 base::Unretained(object.get())),
      base::Bind(&RunningApplicationsManager::OnExported,
                 weak_factory_.GetWeakPtr()));

  object->properties()->Set(
      kRunningApplicationDBusInterface, "AppID",
      scoped_ptr<base::Value>(base::Value::CreateStringValue(app_id)));
  dbus::ObjectPath path = object->path();
  adaptor_.AddManagedObject(object.Pass());
}

}  // namespace application
}  // namespace xwalk

