// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service_provider_linux.h"

#include <string>
#include "base/bind.h"
#include "dbus/bus.h"
#include "dbus/exported_object.h"
#include "dbus/message.h"
#include "xwalk/dbus/xwalk_service_name.h"
#include "xwalk/application/browser/linux/installed_applications_root.h"

namespace xwalk {
namespace application {

ApplicationServiceProviderLinux::ApplicationServiceProviderLinux(
    ApplicationService* app_service)
    : ApplicationServiceProvider(app_service),
      dbus_manager_(kXWalkDBusServiceName) {
  dbus_manager_.Initialize(
      base::Bind(&ApplicationServiceProviderLinux::OnDBusInitialized,
                 base::Unretained(this)));
}

ApplicationServiceProviderLinux::~ApplicationServiceProviderLinux() {}

void ApplicationServiceProviderLinux::OnDBusInitialized() {
  VLOG(1) << "D-Bus initialized.";

  installed_apps_.reset(new InstalledApplicationsRoot(
      dbus_manager_.session_bus(), app_service()));

  // TODO(cmarcelo): This is just a placeholder to test D-Bus is working, remove
  // once we exported proper objects.
  ExportTestObject();
}

namespace {

void OnExported(const std::string& interface_name,
                const std::string& method_name,
                bool success) {
  if (!success) {
    LOG(WARNING) << "Couldn't export method '" + method_name
                 << "' from '" + interface_name + "'.";
  }
}

void OnPing(dbus::MethodCall* method_call,
            dbus::ExportedObject::ResponseSender response_sender) {
  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendString("Pong");
  response_sender.Run(response.Pass());
}

}  // namespace

void ApplicationServiceProviderLinux::ExportTestObject() {
  dbus::ExportedObject* object =
      dbus_manager_.session_bus()->GetExportedObject(dbus::ObjectPath("/test"));
  object->ExportMethod("org.crosswalkproject.TestInterface", "Ping",
                       base::Bind(&OnPing), base::Bind(&OnExported));
}

}  // namespace application
}  // namespace xwalk
