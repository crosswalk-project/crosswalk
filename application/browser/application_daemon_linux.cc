// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_daemon.h"

#include "base/bind.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread.h"
#include "dbus/bus.h"
#include "dbus/exported_object.h"
#include "dbus/message.h"
#include "dbus/object_manager.h"
#include "dbus/object_path.h"
#include "dbus/property.h"
#include "xwalk/application/browser/application_store.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

class ApplicationDBusService {
 public:
  explicit ApplicationDBusService(ApplicationSystem* application_system)
    : origin_thread_id_(base::PlatformThread::CurrentId()),
      system_(application_system),
      dbus_thread_(NULL),
      session_bus_(NULL),
      exported_object_(NULL) {
  }
  ~ApplicationDBusService() {
    if (session_bus_)
      session_bus_->ShutdownOnDBusThreadAndBlock();
  }

  void OnOwnershipAcquired(const std::string& service_name, bool success) {
    DCHECK(OnOriginThread());
    // Should safely exit xwalk here.
    LOG_IF(ERROR, !success) << "Failed to own: " << service_name;
  }

  void OnExported(const std::string& interface_name,
                  const std::string& method_path,
                  bool success) {
    DCHECK(OnOriginThread());
    // Should safely exit xwalk here.
    LOG_IF(ERROR, !success) << "Failed to export: " << interface_name
        << " :: " << method_path;
  }

  // D-Bus function Launch(string id) return (bool success, string message)
  void Launch(dbus::MethodCall* method_call,
               dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    dbus::MessageReader reader(method_call);
    std::string id;
    scoped_ptr<dbus::Response> response =
        dbus::Response::FromMethodCall(method_call);
    dbus::MessageWriter writer(response.get());
    if (!reader.PopString(&id)) {
      response_sender.Run(scoped_ptr<dbus::Response>());
      // Invalid IPC call.
      return;
    }
    if (!xwalk::application::Application::IsIDValid(id)) {
      writer.AppendBool(false);
      writer.AppendString("Invalid application ID: " + id);
      response_sender.Run(response.Pass());
      return;
    }
    xwalk::application::ApplicationService* service =
        system_->application_service();
    LOG(INFO) << "Launching appliation: " << id;
    if (service->Launch(id)) {
      writer.AppendBool(true);
      writer.AppendString("Application ID: " + id + " launched.");
    } else {
      writer.AppendBool(false);
      writer.AppendString("Failed to launch.");
    }
    response_sender.Run(response.Pass());
  }

  // D-Bus function Terminate(string id) return (bool success, string message)
  void Terminate(dbus::MethodCall* method_call,
               dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    dbus::MessageReader reader(method_call);
    std::string id;
    scoped_ptr<dbus::Response> response =
        dbus::Response::FromMethodCall(method_call);
    dbus::MessageWriter writer(response.get());
    if (!reader.PopString(&id)) {
      response_sender.Run(scoped_ptr<dbus::Response>());
      // Invalid IPC call.
      return;
    }
    if (!xwalk::application::Application::IsIDValid(id)) {
      writer.AppendBool(false);
      writer.AppendString("Invalid application ID: " + id);
      response_sender.Run(response.Pass());
      return;
    }
    xwalk::application::ApplicationService* service =
        system_->application_service();
    LOG(INFO) << "Terminating appliation: " << id;
    if (service->Terminate(id)) {
      writer.AppendBool(true);
      writer.AppendString("Application ID: " + id + " Terminated.");
    } else {
      writer.AppendBool(false);
      writer.AppendString("Failed to terminate");
    }
    response_sender.Run(response.Pass());
  }

  // D-Bus function Install(string path) return (bool success, string message)
  void Install(dbus::MethodCall* method_call,
               dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    dbus::MessageReader reader(method_call);
    std::string path;
    scoped_ptr<dbus::Response> response =
        dbus::Response::FromMethodCall(method_call);
    dbus::MessageWriter writer(response.get());
    if (!reader.PopString(&path)) {
      response_sender.Run(scoped_ptr<dbus::Response>());
      // Invalid IPC call.
      return;
    }
    base::FilePath file_path(path);
    std::string id;
    xwalk::application::ApplicationService* service =
        system_->application_service();
    if (service->Install(file_path, &id)) {
      writer.AppendBool(true);
      writer.AppendString("Application ID: " + id + " installed.");
    } else {
      writer.AppendBool(false);
      writer.AppendString("Failed to install: " + path);
    }
    response_sender.Run(response.Pass());
  }

  // D-Bus function Uninstall(string id) return (bool success, string message)
  void Uninstall(dbus::MethodCall* method_call,
               dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    dbus::MessageReader reader(method_call);
    std::string id;
    scoped_ptr<dbus::Response> response =
        dbus::Response::FromMethodCall(method_call);
    dbus::MessageWriter writer(response.get());
    if (!reader.PopString(&id)) {
      response_sender.Run(scoped_ptr<dbus::Response>());
      // Invalid IPC call.
      return;
    }
    xwalk::application::ApplicationService* service =
        system_->application_service();
    if (service->Uninstall(id)) {
      writer.AppendBool(true);
      writer.AppendString("Application ID: " + id + " uninstalled");
    } else {
      writer.AppendBool(false);
      writer.AppendString("Failed to uninstall: " + id);
    }
    response_sender.Run(response.Pass());
  }

  void ListApps(dbus::MethodCall* method_call,
               dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    dbus::MessageReader reader(method_call);
    scoped_ptr<dbus::Response> response =
        dbus::Response::FromMethodCall(method_call);
    dbus::MessageWriter writer(response.get());
    // This function has no parameter.
    xwalk::application::ApplicationService* service =
        system_->application_service();
    xwalk::application::ApplicationStore::ApplicationMap* apps =
            service->GetInstalledApplications();
    std::string ret = "";
    ret += "Application ID                       Application Name\n";
    ret += "-----------------------------------------------------\n";
    xwalk::application::ApplicationStore::ApplicationMapIterator it;
    for (it = apps->begin(); it != apps->end(); ++it) {
      ret += (it->first + "     " + it->second->Name() + "\n");
    }
    ret += "-----------------------------------------------------\n";
    writer.AppendBool(true);
    writer.AppendString(ret);
    response_sender.Run(response.Pass());
  }

  bool StartService() {
    DCHECK(OnOriginThread());
    if (dbus_thread_) {
      LOG(ERROR) << "D-Bus service has already been started.";
      return false;
    }
    // Start the D-Bus thread.
    dbus_thread_ = new base::Thread("D-Bus Thread");
    base::Thread::Options thread_options;
    thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
    if (!dbus_thread_->StartWithOptions(thread_options)) {
      LOG(ERROR) << "Failed to create D-Bus thread.";
      return false;
    }

    // Start the application service.
    dbus::Bus::Options options;
    options.bus_type = dbus::Bus::SESSION;
    options.connection_type = dbus::Bus::PRIVATE;
    options.dbus_task_runner = dbus_thread_->message_loop_proxy();
    session_bus_ = new dbus::Bus(options);
    session_bus_->RequestOwnership(kDbusServiceName,
                      dbus::Bus::REQUIRE_PRIMARY,
                      base::Bind(&ApplicationDBusService::OnOwnershipAcquired,
                      base::Unretained(this)));
    exported_object_ = session_bus_->GetExportedObject(
        dbus::ObjectPath(kDbusObjectPath));
    exported_object_->ExportMethod(
        kDbusAppInterfaceName,
        "Launch",
        base::Bind(&ApplicationDBusService::Launch,
                   base::Unretained(this)),
        base::Bind(&ApplicationDBusService::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDbusAppInterfaceName,
        "Terminate",
        base::Bind(&ApplicationDBusService::Terminate,
                   base::Unretained(this)),
        base::Bind(&ApplicationDBusService::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDbusAppInterfaceName,
        "Install",
        base::Bind(&ApplicationDBusService::Install,
                   base::Unretained(this)),
        base::Bind(&ApplicationDBusService::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDbusAppInterfaceName,
        "Uninstall",
        base::Bind(&ApplicationDBusService::Uninstall,
                   base::Unretained(this)),
        base::Bind(&ApplicationDBusService::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDbusAppInterfaceName,
        "ListApps",
        base::Bind(&ApplicationDBusService::ListApps,
                   base::Unretained(this)),
        base::Bind(&ApplicationDBusService::OnExported,
                   base::Unretained(this)));
    return true;
  }

  private:
    bool OnOriginThread() {
      return base::PlatformThread::CurrentId() == origin_thread_id_;
    }
    ApplicationSystem* system_;
    base::PlatformThreadId origin_thread_id_;
    base::Thread* dbus_thread_;
    dbus::Bus* session_bus_;
    dbus::ExportedObject* exported_object_;
};

static ApplicationDBusService* g_application_dbus_service = NULL;

ApplicationDaemon::ApplicationDaemon(xwalk::RuntimeContext* runtime_context)
  : runtime_context_(runtime_context) {
}

ApplicationDaemon::~ApplicationDaemon() {
  delete g_application_dbus_service;
  g_application_dbus_service = NULL;
}

bool ApplicationDaemon::Start() {
  if (g_application_dbus_service)
    return false;
  LOG(INFO) << "Starting daemon...";
  g_application_dbus_service = new ApplicationDBusService(
      runtime_context_->GetApplicationSystem());
  return g_application_dbus_service->StartService();
}

}  // namespace application
}  // namespace xwalk
