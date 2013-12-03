// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/running_applications_root.h"

#include <string>
#include "base/bind.h"
#include "dbus/bus.h"
#include "dbus/message.h"

namespace {

// D-Bus Interface implemented by the root object of running applications.
//
// Methods:
//
//   Launch(string app_id)
//     Launches the application with 'app_id'.
//
// TODO(cmarcelo): This should return an object path pointing to the
// object representing the running application.
const char kRunningApplicationsRootDBusInterface[] =
    "org.crosswalkproject.RunningApplicationsRoot";

const char kRunningApplicationsRootDBusError[] =
    "org.crosswalkproject.RunningApplicationsRoot.Error";

const dbus::ObjectPath kRunningApplicationsRootPath("/running");

}  // namespace

namespace xwalk {
namespace application {

RunningApplicationsRoot::RunningApplicationsRoot(
    scoped_refptr<dbus::Bus> bus, ApplicationService* service)
    : weak_factory_(this),
      application_service_(service),
      bus_(bus) {
  root_object_ = bus_->GetExportedObject(kRunningApplicationsRootPath);
  root_object_->ExportMethod(
      kRunningApplicationsRootDBusInterface, "Launch",
      base::Bind(&RunningApplicationsRoot::OnLaunch,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&RunningApplicationsRoot::OnExported,
                 weak_factory_.GetWeakPtr()));
}

RunningApplicationsRoot::~RunningApplicationsRoot() {}

namespace {

scoped_ptr<dbus::Response> CreateError(dbus::MethodCall* method_call,
                                       const std::string& message) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kRunningApplicationsRootDBusError, message);
    return error_response.PassAs<dbus::Response>();
}

}

void RunningApplicationsRoot::OnLaunch(
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

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}

void RunningApplicationsRoot::OnExported(
    const std::string& interface_name,
    const std::string& method_name,
    bool success) {
  if (!success) {
    LOG(WARNING) << "Error exporting method '" << interface_name
                 << "." << method_name << "' in '"
                 << kRunningApplicationsRootPath.value() << "'.";
  }
}

}  // namespace application
}  // namespace xwalk

