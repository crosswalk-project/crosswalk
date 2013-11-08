// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service_provider.h"

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/strings/string_util.h"
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

class ApplicationServiceProviderImpl {
 public:
  explicit ApplicationServiceProviderImpl(ApplicationSystem* application_system)
    : origin_thread_id_(base::PlatformThread::CurrentId()),
      system_(application_system),
      exported_object_(NULL) {
  }
  ~ApplicationServiceProviderImpl() {
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

  // D-Bus function Launch(string id) return (bool success)
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
      LOG(ERROR) << "Invalid D-Bus Message.";
      return;
    }

    if (!xwalk::application::Application::IsIDValid(id)) {
      writer.AppendBool(false);
      LOG(ERROR) << "Invalid application ID: " << id;
      response_sender.Run(response.Pass());
      return;
    }

    xwalk::application::ApplicationService* service =
        system_->application_service();

    if (service->Launch(id)) {
      writer.AppendBool(true);
      LOG(INFO) << "Application with ID: " << id << " was launched.";
    } else {
      writer.AppendBool(false);
      LOG(ERROR) << "Failed to launch application with ID " << id;
    }
    response_sender.Run(response.Pass());
  }

  // D-Bus function Terminate(string id) return (bool success)
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
      LOG(ERROR) << "Invalid D-Bus Message.";
      return;
    }

    if (!xwalk::application::Application::IsIDValid(id)) {
      writer.AppendBool(false);
      LOG(ERROR) << "Invalid application ID: " << id;
      response_sender.Run(response.Pass());
      return;
    }
    xwalk::application::ApplicationService* service =
        system_->application_service();

    if (service->Terminate(id)) {
      writer.AppendBool(true);
      LOG(INFO) << "Application with ID: " << id << " was terminated.";
    } else {
      writer.AppendBool(false);
      LOG(ERROR) << "Failed to terminate";
    }
    response_sender.Run(response.Pass());
  }

  // D-Bus function Install(string path) return (bool success)
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
      LOG(ERROR) << "Invalid D-Bus Message.";
      return;
    }
    base::FilePath file_path(path);
    std::string id;
    xwalk::application::ApplicationService* service =
        system_->application_service();

    if (service->Install(file_path, &id)) {
      writer.AppendBool(true);
      LOG(INFO) << "Application ID: " << id << " installed.";
    } else {
      writer.AppendBool(false);
      LOG(ERROR) << "Failed to install: " << path;
    }
    response_sender.Run(response.Pass());
  }

  // D-Bus function Uninstall(string id) return (bool success)
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
      LOG(ERROR) << "Invalid D-Bus Message.";
      return;
    }
    xwalk::application::ApplicationService* service =
        system_->application_service();

    if (service->Uninstall(id)) {
      writer.AppendBool(true);
      LOG(INFO) << "Application ID: " << id << " uninstalled";
    } else {
      writer.AppendBool(false);
      LOG(ERROR) << "Failed to uninstall: " << id;
    }
    response_sender.Run(response.Pass());
  }

  // D-Bus function ListInstalledApps() return (bool success, string app_list)
  // The app_list is a JSON array, each element of which is a ID-name bundle.
  // For example [{123456,app1},{234567,app2},{345678,app3}]
  void ListInstalledApps(dbus::MethodCall* method_call,
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
    std::vector<std::string> app_list;
    xwalk::application::ApplicationStore::ApplicationMapIterator it;
    for (it = apps->begin(); it != apps->end(); ++it) {
      app_list.push_back("{" + it->first + "," + it->second->Name() + "}");
    }
    std::string ret = "[" + JoinString(app_list, ',') + "]";
    writer.AppendBool(true);
    writer.AppendString(ret);
    response_sender.Run(response.Pass());
  }

  bool StartService() {
    DCHECK(OnOriginThread());
    if (dbus_thread_.get()) {
      LOG(ERROR) << "D-Bus service has already been started.";
      return false;
    }
    // Start the D-Bus thread.
    dbus_thread_.reset(
            new base::Thread("Crosswalk Application D-Bus Service Thread"));
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
    session_bus_->RequestOwnership(kDBusServiceName,
                      dbus::Bus::REQUIRE_PRIMARY,
                      base::Bind(
                          &ApplicationServiceProviderImpl::OnOwnershipAcquired,
                      base::Unretained(this)));
    exported_object_ = session_bus_->GetExportedObject(
        dbus::ObjectPath(kDBusObjectPath));
    exported_object_->ExportMethod(
        kDBusAppInterfaceName,
        kDBusMethodLaunch,
        base::Bind(&ApplicationServiceProviderImpl::Launch,
                   base::Unretained(this)),
        base::Bind(&ApplicationServiceProviderImpl::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDBusAppInterfaceName,
        kDBusMethodTerminate,
        base::Bind(&ApplicationServiceProviderImpl::Terminate,
                   base::Unretained(this)),
        base::Bind(&ApplicationServiceProviderImpl::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDBusAppInterfaceName,
        kDBusMethodInstall,
        base::Bind(&ApplicationServiceProviderImpl::Install,
                   base::Unretained(this)),
        base::Bind(&ApplicationServiceProviderImpl::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDBusAppInterfaceName,
        kDBusMethodUninstall,
        base::Bind(&ApplicationServiceProviderImpl::Uninstall,
                   base::Unretained(this)),
        base::Bind(&ApplicationServiceProviderImpl::OnExported,
                   base::Unretained(this)));
    exported_object_->ExportMethod(
        kDBusAppInterfaceName,
        kDBusMethodListInstalledApps,
        base::Bind(&ApplicationServiceProviderImpl::ListInstalledApps,
                   base::Unretained(this)),
        base::Bind(&ApplicationServiceProviderImpl::OnExported,
                   base::Unretained(this)));
    return true;
  }

  private:
    const bool OnOriginThread() {
      return base::PlatformThread::CurrentId() == origin_thread_id_;
    }
    ApplicationSystem* system_;
    base::PlatformThreadId origin_thread_id_;
    dbus::ExportedObject* exported_object_;
    scoped_ptr<base::Thread> dbus_thread_;
    scoped_refptr<dbus::Bus> session_bus_;
};

ApplicationServiceProvider::ApplicationServiceProvider(
        xwalk::RuntimeContext* runtime_context)
  : runtime_context_(runtime_context),
    impl_(NULL) {
}

ApplicationServiceProvider::~ApplicationServiceProvider() {
  delete impl_;
}

bool ApplicationServiceProvider::Start() {
  LOG(INFO) << "Starting D-Bus service...";
  impl_ = new ApplicationServiceProviderImpl(
      runtime_context_->GetApplicationSystem());
  return impl_->StartService();
}

}  // namespace application
}  // namespace xwalk
