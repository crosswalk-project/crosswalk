// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/running_application_object.h"

#include <string>
#include "base/values.h"
#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/exported_object.h"
#include "xwalk/application/browser/application_tizen.h"
#include "xwalk/application/browser/linux/running_applications_manager.h"

namespace {

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
    "org.crosswalkproject.Running.Application1";

const char kRunningApplicationDBusError[] =
    "org.crosswalkproject.Running.Application.Error";


}  // namespace

namespace xwalk {
namespace application {

RunningApplicationObject::RunningApplicationObject(
    scoped_refptr<dbus::Bus> bus,
    const std::string& app_id,
    const std::string& launcher_name,
    Application* application)
    : dbus::ManagedObject(bus, GetRunningPathForAppID(app_id)),
      bus_(bus),
      launcher_name_(launcher_name),
      application_(application) {
  ListenForOwnerChange();

  properties()->Set(
      kRunningApplicationDBusInterface, "AppID",
      scoped_ptr<base::Value>(base::Value::CreateStringValue(app_id)));

  dbus_object()->ExportMethod(
      kRunningApplicationDBusInterface, "Terminate",
      base::Bind(&RunningApplicationObject::OnTerminate,
                 base::Unretained(this)),
      base::Bind(&RunningApplicationObject::OnExported,
                 base::Unretained(this)));

  dbus_object()->ExportMethod(
      kRunningApplicationDBusInterface, "GetEPChannel",
      base::Bind(&RunningApplicationObject::OnGetExtensionProcessChannel,
                 base::Unretained(this)),
      base::Bind(&RunningApplicationObject::OnExported,
                 base::Unretained(this)));

#if defined(OS_TIZEN)
  dbus_object()->ExportMethod(
      kRunningApplicationDBusInterface, "Hide",
      base::Bind(&RunningApplicationObject::OnHide,
                 base::Unretained(this)),
      base::Bind(&RunningApplicationObject::OnExported,
                 base::Unretained(this)));
#endif
}

RunningApplicationObject::~RunningApplicationObject() {
  UnlistenForOwnerChange();
}

void RunningApplicationObject::TerminateApplication() {
  application_->Terminate();
}

void RunningApplicationObject::OnExported(const std::string& interface_name,
                                          const std::string& method_name,
                                          bool success) {
  if (!success) {
    LOG(WARNING) << "Error exporting method '" << interface_name
                 << "." << method_name << "' in '"
                 << path().value() << "'.";
  }
}

void RunningApplicationObject::OnTerminate(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  // We only allow the caller of Launch() to call Terminate().
  if (method_call->GetSender() != launcher_name_) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(method_call,
                                            kRunningApplicationDBusError,
                                            "Not permitted");
    response_sender.Run(error_response.PassAs<dbus::Response>());
    return;
  }

  TerminateApplication();

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}

void RunningApplicationObject::OnGetExtensionProcessChannel(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&RunningApplicationObject::SendChannel,
                 base::Unretained(this),
                 method_call,
                 response_sender));
}

#if defined(OS_TIZEN)
void RunningApplicationObject::OnHide(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  if (method_call->GetSender() != launcher_name_) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(method_call,
                                            kRunningApplicationDBusError,
                                            "Not permitted");
    response_sender.Run(error_response.PassAs<dbus::Response>());
    return;
  }

  ToApplicationTizen(application_)->Hide();

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}
#endif

void RunningApplicationObject::ListenForOwnerChange() {
  owner_change_callback_ =
      base::Bind(&RunningApplicationObject::OnNameOwnerChanged,
                 base::Unretained(this));
  bus_->ListenForServiceOwnerChange(launcher_name_, owner_change_callback_);
}

void RunningApplicationObject::UnlistenForOwnerChange() {
  if (owner_change_callback_.is_null())
    return;
  bus_->UnlistenForServiceOwnerChange(launcher_name_, owner_change_callback_);
  owner_change_callback_.Reset();
}

void RunningApplicationObject::OnNameOwnerChanged(
    const std::string& service_owner) {
  if (service_owner.empty()) {
    // The process that sent the 'Launch' message has exited the session bus,
    // we should kill the Running Application.
    OnLauncherDisappeared();
  }
}

void RunningApplicationObject::OnLauncherDisappeared() {
  TerminateApplication();
}

void RunningApplicationObject::SendChannel(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);

  int fd = ep_bp_channel_.socket.fd;
  if (fd == -1) {  // EP was not yet created, return empty response.
    response_sender.Run(response.Pass());
    return;
  }

  dbus::MessageWriter writer(response.get());
  writer.AppendString(ep_bp_channel_.name);

  scoped_ptr<dbus::FileDescriptor> client_fd(new dbus::FileDescriptor(fd));
  client_fd->CheckValidity();
  CHECK(client_fd->is_valid());
  writer.AppendFileDescriptor(*client_fd);

  response_sender.Run(response.Pass());
}

void RunningApplicationObject::ExtensionProcessCreated(
    const IPC::ChannelHandle& handle) {
  ep_bp_channel_ = handle;
  dbus::Signal signal(kRunningApplicationDBusInterface, "EPChannelCreated");
  dbus_object()->SendSignal(&signal);
}

}  // namespace application
}  // namespace xwalk
