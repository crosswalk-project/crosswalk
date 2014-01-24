// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/running_applications_manager.h"

#include <string>
#include "base/bind.h"
#include "dbus/bus.h"
#include "dbus/message.h"

#include "xwalk/application/browser/linux/running_application_object.h"

namespace {

// D-Bus Interface implemented by the manager object of running applications.
//
// Methods:
//
//   Launch(string app_id) -> ObjectPath
//     Launches the application with 'app_id'.
const char kRunningManagerDBusInterface[] =
    "org.crosswalkproject.Running.Manager1";

const char kRunningManagerDBusError[] =
    "org.crosswalkproject.Running.Manager.Error";

const dbus::ObjectPath kRunningManagerDBusPath("/running1");


}  // namespace

namespace xwalk {
namespace application {

dbus::ObjectPath GetRunningPathForAppID(const std::string& app_id) {
  return dbus::ObjectPath(kRunningManagerDBusPath.value() + "/" + app_id);
}

RunningApplicationsManager::RunningApplicationsManager(
    scoped_refptr<dbus::Bus> bus, ApplicationService* service)
    : weak_factory_(this),
      application_service_(service),
      adaptor_(bus, kRunningManagerDBusPath) {
  application_service_->AddObserver(this);

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
  unsigned int launcher_pid;
  if (!reader.PopString(&app_id) ||
      !reader.PopUint32(&launcher_pid)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error parsing message. Missing arguments.");
    response_sender.Run(response.Pass());
    return;
  }

  Application::LaunchParams params;
  params.launcher_pid = launcher_pid;

  Application* application = application_service_->Launch(app_id, params);
  if (!application) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error launching application with id " + app_id);
    response_sender.Run(response.Pass());
    return;
  }

  // FIXME(cmarcelo): ApplicationService will tell us when new applications
  // appear (with DidLaunchApplication()) and we create new managed objects
  // in D-Bus based on that.
  dbus::ObjectPath path = AddObject(app_id, method_call->GetSender(),
                                    application);

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendObjectPath(path);
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

void RunningApplicationsManager::WillDestroyApplication(Application* app) {
  dbus::ObjectPath path = GetRunningPathForAppID(app->id());

  adaptor_.RemoveManagedObject(path);
}

dbus::ObjectPath RunningApplicationsManager::AddObject(
    const std::string& app_id, const std::string& launcher_name,
    Application* application) {
  scoped_ptr<RunningApplicationObject> running_application(
      new RunningApplicationObject(adaptor_.bus(), app_id,
                                   launcher_name, application));

  dbus::ObjectPath path = running_application->path();

  adaptor_.AddManagedObject(running_application.PassAs<dbus::ManagedObject>());

  return path;
}

}  // namespace application
}  // namespace xwalk
