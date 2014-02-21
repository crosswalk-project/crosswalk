// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/running_application_object_tizen.h"

namespace {

// D-Bus Interface implemented for forwarding application command from extension
// process to xwalk-launcher.
// TODO(xiang): add permission check for methods on this interface.
const char kTizenAppCmdForwarderInterface[] =
    "org.crosswalkproject.Running.TizenAppCmdForwarder1";

}  // namespace

namespace xwalk {
namespace application {

TizenRunningApplicationObject::TizenRunningApplicationObject(
    scoped_refptr<dbus::Bus> bus,
    const std::string& app_id,
    const std::string& launcher_name,
    Application* application)
    : RunningApplicationObject(bus, app_id, launcher_name, application) {
  dbus_object()->ExportMethod(
      kRunningApplicationDBusInterface, "Hide",
      base::Bind(&TizenRunningApplicationObject::OnHide,
                 base::Unretained(this)),
      base::Bind(&RunningApplicationObject::OnExported,
                 base::Unretained(this)));

  dbus_object()->ExportMethod(
      kTizenAppCmdForwarderInterface, "Launch",
      base::Bind(&TizenRunningApplicationObject::OnLaunchCmd,
                 base::Unretained(this)),
      base::Bind(&RunningApplicationObject::OnExported,
                 base::Unretained(this)));
  dbus_object()->ExportMethod(
      kTizenAppCmdForwarderInterface, "Exit",
      base::Bind(&TizenRunningApplicationObject::OnExitCmd,
                 base::Unretained(this)),
      base::Bind(&RunningApplicationObject::OnExported,
                 base::Unretained(this)));
  dbus_object()->ExportMethod(
      kTizenAppCmdForwarderInterface, "Kill",
      base::Bind(&TizenRunningApplicationObject::OnKillCmd,
                 base::Unretained(this)),
      base::Bind(&RunningApplicationObject::OnExported,
                 base::Unretained(this)));
}

void TizenRunningApplicationObject::OnHide(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  // TODO(xiang): add root window hide impl.
}

void TizenRunningApplicationObject::OnLaunchCmd(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  dbus::MessageReader reader(method_call);
  std::string app_id;
  if (!reader.PopString(&app_id)) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(method_call,
                                            kTizenAppCmdForwarderInterface,
                                            "Invalid arguments.");
    response_sender.Run(error_response.PassAs<dbus::Response>());
    return;
  }

  dbus::Signal signal(kTizenAppCmdForwarderInterface, "OnLaunchCmd");
  dbus::MessageWriter writer(&signal);
  writer.AppendString(app_id);
  dbus_object()->SendSignal(&signal);

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}

void TizenRunningApplicationObject::OnExitCmd(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  dbus::Signal signal(kTizenAppCmdForwarderInterface, "OnExitCmd");
  dbus::MessageWriter writer(&signal);
  dbus_object()->SendSignal(&signal);

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}

void TizenRunningApplicationObject::OnKillCmd(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  dbus::MessageReader reader(method_call);
  std::string app_context_id;
  if (!reader.PopString(&app_context_id)) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(method_call,
                                            kTizenAppCmdForwarderInterface,
                                            "Invalid arguments.");
    response_sender.Run(error_response.PassAs<dbus::Response>());
    return;
  }

  dbus::Signal signal(kTizenAppCmdForwarderInterface, "OnKillCmd");
  dbus::MessageWriter writer(&signal);
  writer.AppendString(app_context_id);
  dbus_object()->SendSignal(&signal);

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}

}  // namespace application
}  // namespace xwalk
