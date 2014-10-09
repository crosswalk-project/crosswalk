// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/linux/dbus_object_manager.h"

#include "base/message_loop/message_loop.h"

namespace {

const char kServiceName[] = "org.crosswalkproject.Runtime1";
const char kRunningManagerIface[] = "org.crosswalkproject.Running.Manager1";
const char kRunningAppIface[] = "org.crosswalkproject.Running.Application1";
const char kRunningManagerDBusPath[] = "/running1";
const char kEPChannelCreatedSignalName[] = "EPChannelCreated";

struct Properties : public dbus::PropertySet {
  dbus::Property<std::string> app_id;

  Properties(dbus::ObjectProxy* object_proxy,
             const std::string& interface_name,
             PropertyChangedCallback property_changed_callback)
      : PropertySet(object_proxy, interface_name, property_changed_callback) {
    RegisterProperty("AppID", &app_id);
  }
};

}  // namespace

DBusObjectManager::DBusObjectManager(dbus::Bus* bus,
                                     base::MessageLoop* main_loop)
    : bus_(bus),
      main_loop_(main_loop),
      weak_ptr_factory_(this) {
  ConnectToApplicationManager();
}

bool DBusObjectManager::Launch(const std::string& appid_or_url,
     int launcher_pid, bool fullscreen, bool remote_debugging) {
  if (!running_proxy_)
    return false;
  dbus::MethodCall method_call(
      kRunningManagerIface, "Launch");
  dbus::MessageWriter writer(&method_call);
  writer.AppendString(appid_or_url);
  writer.AppendUint32(launcher_pid);
  writer.AppendBool(fullscreen);
  writer.AppendBool(remote_debugging);
  scoped_ptr<dbus::Response> response(
      running_proxy_->CallMethodAndBlock(&method_call,
      dbus::ObjectProxy::TIMEOUT_USE_DEFAULT));
  if (!response.get())
    return false;
  if (!response->GetErrorName().empty()) {
    LOG(ERROR) << "Error during call to 'Launch': "
               << response->GetErrorName();
    return false;
  }

  dbus::MessageReader reader(response.get());
  dbus::ObjectPath running_application_path;
  if (!reader.PopObjectPath(&running_application_path)) {
    LOG(WARNING) << "Failed to create app proxy.";
  } else {
    app_proxy_ = bus_->GetObjectProxy(kServiceName, running_application_path);
    if (app_proxy_)
      ConnectToApplicationSignal(kEPChannelCreatedSignalName);
  }
  return true;
}

std::pair<std::string, int> DBusObjectManager::GetEPChannel() const {
  std::pair<std::string, int> fd;
  if (!app_proxy_) {
    fd.second = -1;
    return fd;
  }
  dbus::MethodCall method_call(kRunningAppIface, "GetEPChannel");
  scoped_ptr<dbus::Response> response = app_proxy_->CallMethodAndBlock(
      &method_call, dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
  const std::string& error = response->GetErrorName();
  if (!error.empty()) {
    LOG(ERROR) << "Error during call to 'GetEPChannel': "
               << error;
    fd.second = -1;
    return fd;
  }
  dbus::MessageReader reader(response.release());
  dbus::FileDescriptor extension_process_fd_;
  if (!reader.PopString(&fd.first) ||
      !reader.PopFileDescriptor(&extension_process_fd_)) {
    LOG(ERROR) << "Couldn't get EP Channel";
    fd.second = -1;
    return fd;
  }
  extension_process_fd_.CheckValidity();
  fd.second = extension_process_fd_.TakeValue();
  return fd;
}

bool DBusObjectManager::Suspend() {
  if (!app_proxy_)
    return false;
  dbus::MethodCall method_call(kRunningAppIface, "Suspend");
  scoped_ptr<dbus::Response> response = app_proxy_->CallMethodAndBlock(
      &method_call, dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
  if (!response->GetErrorName().empty()) {
    LOG(ERROR) << "Error during call to 'Suspend': "
               << response->GetErrorName();
    return false;
  }
  return true;
}

bool DBusObjectManager::Resume() {
  if (!app_proxy_)
    return false;
  dbus::MethodCall method_call(kRunningAppIface, "Resume");
  scoped_ptr<dbus::Response> response = app_proxy_->CallMethodAndBlock(
      &method_call, dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
  if (!response->GetErrorName().empty()) {
    LOG(ERROR) << "Error during call to 'Resume': "
               << response->GetErrorName();
    return false;
  }
  return true;
}

void DBusObjectManager::OnOwnershipCallback(const std::string& service_name,
                                            bool success) {
  LOG(WARNING) << "Couldn't get ownership of D-Bus service name: "
               << service_name << ".";
}

void DBusObjectManager::ConnectToApplicationManager() {
  running_apps_manager_ = bus_->GetObjectManager(kServiceName,
      dbus::ObjectPath(kRunningManagerDBusPath));
  running_apps_manager_->RegisterInterface(kRunningAppIface, this);
  running_proxy_ = bus_->GetObjectProxy(kServiceName,
      dbus::ObjectPath(kRunningManagerDBusPath));
}

void DBusObjectManager::ObjectAdded(const dbus::ObjectPath& object_path,
    const std::string& interface_name) {
}

void DBusObjectManager::ObjectRemoved(const dbus::ObjectPath& object_path,
    const std::string& interface_name) {
  if (object_path != app_proxy_->object_path())
    return;
  LOG(INFO) << "Application '" << object_path.value()
            << "' disappeared, exiting.";
  main_loop_->QuitNow();
}

dbus::PropertySet* DBusObjectManager::CreateProperties(
    dbus::ObjectProxy *object_proxy,
    const dbus::ObjectPath& object_path,
    const std::string& interface_name) {
  Properties* properties = new Properties(
      object_proxy, interface_name,
      base::Bind(&DBusObjectManager::OnPropertyChanged,
                 base::Unretained(this), object_path));
  return static_cast<dbus::PropertySet*>(properties);
}

void DBusObjectManager::OnPropertyChanged(const dbus::ObjectPath& object_path,
    const std::string& name) {
  if (!running_apps_manager_)
    ConnectToApplicationManager();
}

void DBusObjectManager::ConnectToApplicationSignal(
    const std::string& signal_name) {
  DCHECK(app_proxy_);
  app_proxy_->ConnectToSignal(kRunningAppIface, signal_name,
      base::Bind(&DBusObjectManager::OnAppSignal,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&DBusObjectManager::OnAppSignalConnected,
                 weak_ptr_factory_.GetWeakPtr()));
}

bool DBusObjectManager::IsApplicationRunning(const std::string& app_id) {
  std::vector<dbus::ObjectPath> objects = running_apps_manager_->GetObjects();
  bool is_running = false;
  for (dbus::ObjectPath obj : objects) {
    Properties* properties =
        static_cast<Properties*>(
            running_apps_manager_->GetProperties(
                obj, kRunningAppIface));
    if (!properties)
      continue;
    if (properties->app_id.value() == app_id) {
      is_running = true;
      break;
    }
  }
  LOG(INFO) << "Application " << app_id << " is "
            << (is_running ? "running." : "not running.");
  return is_running;
}

void DBusObjectManager::OnAppSignal(dbus::Signal* signal) {
  std::string signal_name = signal->GetMember();
  if (signal_name == kEPChannelCreatedSignalName) {
    if (observer_)
      observer_->OnEPChannelCreated();
  } else {
    LOG(INFO) << "Unknown signal received: " << signal_name;
  }
}

void DBusObjectManager::OnAppSignalConnected(
    const std::string& interface_name,
    const std::string& signal_name,
    bool success) {
  if (!success)
    LOG(WARNING) << "Failed to connect signal: " << signal_name;
}
