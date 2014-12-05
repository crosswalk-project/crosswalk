// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/running_applications_manager.h"

#include <string>
#include "base/bind.h"
#include "dbus/bus.h"
#include "dbus/message.h"

#include "xwalk/application/browser/linux/running_application_object.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

#if defined(OS_TIZEN)
#include "xwalk/application/browser/application_service_tizen.h"
#endif

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

// The [tizen_app_id] contains a dot, making it an invalid object path.
// For this reason we replace it with an underscore '_'.
std::string GetAppObjectPathFromAppId(const std::string& app_id) {
#if defined(OS_TIZEN)
  std::string ret(app_id);
  std::replace(ret.begin(), ret.end(), '.', '_');
  return ret;
#else
  return app_id;
#endif
}

dbus::ObjectPath GetRunningPathForAppID(const std::string& app_id) {
  return dbus::ObjectPath(kRunningManagerDBusPath.value() + "/" +
                          GetAppObjectPathFromAppId(app_id));
}

RunningApplicationsManager::RunningApplicationsManager(
    scoped_refptr<dbus::Bus> bus, ApplicationService* service)
    : weak_factory_(this),
      application_service_(service),
      adaptor_(bus, kRunningManagerDBusPath) {
  application_service_->AddObserver(this);

  adaptor_.manager_object()->ExportMethod(
      kRunningManagerDBusInterface, "EnableRemoteDebugging",
      base::Bind(&RunningApplicationsManager::OnRemoteDebuggingEnabled,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&RunningApplicationsManager::OnExported,
                 weak_factory_.GetWeakPtr()));

  adaptor_.manager_object()->ExportMethod(
      kRunningManagerDBusInterface, "Launch",
      base::Bind(&RunningApplicationsManager::OnLaunch,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&RunningApplicationsManager::OnExported,
                 weak_factory_.GetWeakPtr()));

  adaptor_.manager_object()->ExportMethod(
      kRunningManagerDBusInterface, "TerminateIfRunning",
      base::Bind(&RunningApplicationsManager::OnTerminateIfRunning,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&RunningApplicationsManager::OnExported,
                 weak_factory_.GetWeakPtr()));
}

RunningApplicationsManager::~RunningApplicationsManager() {}

RunningApplicationObject* RunningApplicationsManager::GetRunningApp(
    const std::string& app_id) {
  dbus::ManagedObject* managed_object =
      adaptor_.GetManagedObject(GetRunningPathForAppID(app_id));
  DCHECK(managed_object);
  return static_cast<RunningApplicationObject*>(managed_object);
}

namespace {

scoped_ptr<dbus::Response> CreateError(dbus::MethodCall* method_call,
                                       const std::string& message) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kRunningManagerDBusError, message);
    return error_response.Pass();
}

}  // namespace

void RunningApplicationsManager::OnRemoteDebuggingEnabled(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  dbus::MessageReader reader(method_call);
  unsigned int debugging_port;

  if (!reader.PopUint32(&debugging_port)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error parsing message. Missing arguments.");
    response_sender.Run(response.Pass());
    return;
  }

  if (debugging_port != 0) {
    XWalkRunner::GetInstance()->EnableRemoteDebugging(debugging_port);
  } else {
    XWalkRunner::GetInstance()->DisableRemoteDebugging();
  }

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendUint32(debugging_port);
  response_sender.Run(response.Pass());
}

void RunningApplicationsManager::OnLaunch(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {

  dbus::MessageReader reader(method_call);
  std::string app_id_or_url;
  // We might want to pass key-value pairs if have more parameters in future.
  unsigned int launcher_pid;
  bool fullscreen;
  bool remote_debugging;

  if (!reader.PopString(&app_id_or_url) ||
      !reader.PopUint32(&launcher_pid) ||
      !reader.PopBool(&fullscreen) ||
      !reader.PopBool(&remote_debugging)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error parsing message. Missing arguments.");
    response_sender.Run(response.Pass());
    return;
  }

  Application::LaunchParams params;
  params.launcher_pid = launcher_pid;
  params.force_fullscreen = fullscreen;
  params.remote_debugging = remote_debugging;

  Application* application = NULL;
  GURL url(app_id_or_url);
  if (!url.spec().empty())
    application = application_service_->LaunchHostedURL(url, params);

#if defined(OS_TIZEN)
  if (!application)
    application = ToApplicationServiceTizen(
        application_service_)->LaunchFromAppID(app_id_or_url, params);
#endif

  if (!application) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error launching application with id or url"
                    + app_id_or_url);
    response_sender.Run(response.Pass());
    return;
  }

  // FIXME(cmarcelo): ApplicationService will tell us when new applications
  // appear (with DidLaunchApplication()) and we create new managed objects
  // in D-Bus based on that.
  dbus::ObjectPath path =
      AddObject(GetAppObjectPathFromAppId(application->id()),
                method_call->GetSender(),
                application);

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendObjectPath(path);
  response_sender.Run(response.Pass());
}

void RunningApplicationsManager::OnTerminateIfRunning(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {

  dbus::MessageReader reader(method_call);
  std::string app_id;

  if (!reader.PopString(&app_id)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error parsing message. Missing argument.");
    response_sender.Run(response.Pass());
    return;
  }

  if (Application* app = application_service_->GetApplicationByID(app_id)) {
    CHECK(app_id == app->id());
    app->Terminate();
  }

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

  adaptor_.AddManagedObject(running_application.Pass());

  return path;
}

}  // namespace application
}  // namespace xwalk
