// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/installed_applications_root.h"

#include <algorithm>
#include <string>
#include "base/bind.h"
#include "base/files/file_path.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "xwalk/application/browser/application_store.h"
#include "xwalk/dbus/property_exporter.h"
#include "xwalk/dbus/xwalk_service_name.h"
#include "xwalk/application/browser/linux/installed_application_object.h"

namespace {

// D-Bus Interface implemented by the root object of installed applications.
//
// Methods:
//
//   Install(string path) -> ObjectPath
//     Will install application at "path", that should be an absolute path to
//     the package file. If installation is successful, returns the ObjectPath
//     of the InstalledApplication object that represents it.
const char kInstalledApplicationsRootDBusInterface[] =
    "org.crosswalkproject.InstalledApplicationsRoot";

const char kInstalledApplicationsRootDBusError[] =
    "org.crosswalkproject.InstalledApplicationsRoot.Error";

const dbus::ObjectPath kInstalledApplicationsRootPath("/installed");

}  // namespace

namespace xwalk {
namespace application {

// TODO(cmarcelo): Extract the ObjectManager bits into an ObjectManager<T> class
// and make InstalledApplicationsRoot a subclass of
// ObjectManager<InstalledApplicationObject>. The interface for T expects a
// PropertyExporter to be available.
InstalledApplicationsRoot::InstalledApplicationsRoot(
    scoped_refptr<dbus::Bus> bus, ApplicationService* service)
    : weak_factory_(this),
      application_service_(service),
      bus_(bus) {
  application_service_->AddObserver(this);

  root_object_ = bus_->GetExportedObject(kInstalledApplicationsRootPath);
  root_object_->ExportMethod(
      "org.freedesktop.DBus.ObjectManager", "GetManagedObjects",
      base::Bind(&InstalledApplicationsRoot::OnGetManagedObjects,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&InstalledApplicationsRoot::OnExported,
                 weak_factory_.GetWeakPtr()));

  root_object_->ExportMethod(
      kInstalledApplicationsRootDBusInterface, "Install",
      base::Bind(&InstalledApplicationsRoot::OnInstall,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&InstalledApplicationsRoot::OnExported,
                 weak_factory_.GetWeakPtr()));

  CreateInitialObjects();
}

InstalledApplicationsRoot::~InstalledApplicationsRoot() {
  application_service_->RemoveObserver(this);
}

void InstalledApplicationsRoot::OnApplicationInstalled(
    const std::string& app_id) {
  scoped_refptr<const Application> app(
      application_service_->GetApplicationByID(app_id));
  installed_apps_.push_back(CreateObject(app));

  // TODO(cmarcelo): Emit InterfacesAdded() signal.
}

namespace {

struct MatchAppID {
 public:
  explicit MatchAppID(const std::string& app_id) : app_id_(app_id) {}
  bool operator()(const InstalledApplicationObject* object) {
    return app_id_ == object->app_id();
  }
  const std::string app_id_;
};

}  // namespace

void InstalledApplicationsRoot::OnApplicationUninstalled(
    const std::string& app_id) {
  ScopedVector<InstalledApplicationObject>::iterator it = std::find_if(
      installed_apps_.begin(), installed_apps_.end(), MatchAppID(app_id));
  if (it == installed_apps_.end()) {
    LOG(WARNING) << "Notified about uninstallation of unknown app_id.";
    return;
  }

  // We need to explicitly unregister the exported object.
  bus_->UnregisterExportedObject((*it)->path());

  // Since this is a ScopedVector, erasing will actually destroy the value.
  installed_apps_.erase(it);

  // TODO(cmarcelo): Emit InterfacesRemoved() signal.
}

void InstalledApplicationsRoot::CreateInitialObjects() {
  ApplicationStore::ApplicationMap* apps =
      application_service_->GetInstalledApplications();
  ApplicationStore::ApplicationMap::iterator it;
  for (it = apps->begin(); it != apps->end(); ++it)
    installed_apps_.push_back(CreateObject(it->second));
}

InstalledApplicationObject* InstalledApplicationsRoot::CreateObject(
    scoped_refptr<const Application> app) {
  InstalledApplicationObject* object =
      new InstalledApplicationObject(bus_, kInstalledApplicationsRootPath, app);
  // See comment in InstalledApplicationsRoot::OnUninstall().
  object->ExportUninstallMethod(
      base::Bind(&InstalledApplicationsRoot::OnUninstall,
                 weak_factory_.GetWeakPtr(),
                 base::Unretained(object)),
      base::Bind(&InstalledApplicationsRoot::OnExported,
                 weak_factory_.GetWeakPtr()));
  return object;
}

void InstalledApplicationsRoot::OnGetManagedObjects(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());

  dbus::MessageWriter dict_writer(NULL);
  writer.OpenArray("{oa{sa{sv}}}", &dict_writer);

  ScopedVector<InstalledApplicationObject>::const_iterator it;
  for (it = installed_apps_.begin(); it != installed_apps_.end(); ++it) {
    InstalledApplicationObject* installed_app = *it;
    dbus::MessageWriter entry_writer(NULL);
    dict_writer.OpenDictEntry(&entry_writer);
    entry_writer.AppendObjectPath(installed_app->path());
    installed_app->AppendAllPropertiesToWriter(&entry_writer);
    dict_writer.CloseContainer(&entry_writer);
  }

  writer.CloseContainer(&dict_writer);
  response_sender.Run(response.Pass());
}

namespace {

scoped_ptr<dbus::Response> CreateError(dbus::MethodCall* method_call,
                                       const std::string& message) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kInstalledApplicationsRootDBusError, message);
    return error_response.PassAs<dbus::Response>();
}

}  // namespace

void InstalledApplicationsRoot::OnInstall(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  dbus::MessageReader reader(method_call);
  std::string file_path_str;
  if (!reader.PopString(&file_path_str)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call, "Error parsing message.");
    response_sender.Run(response.Pass());
    return;
  }

  const base::FilePath file_path(file_path_str);
  if (!file_path.IsAbsolute()) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call, "Path to install must be absolute.");
    response_sender.Run(response.Pass());
    return;
  }

  std::string app_id;
  if (!application_service_->Install(file_path, &app_id)) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error installing application with path: " + file_path_str);
    response_sender.Run(response.Pass());
    return;
  }

  ScopedVector<InstalledApplicationObject>::iterator it = std::find_if(
      installed_apps_.begin(), installed_apps_.end(), MatchAppID(app_id));
  CHECK(it != installed_apps_.end());

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendObjectPath((*it)->path());
  response_sender.Run(response.Pass());
}

// InstalledApplicationsRoot implements the callback exposed in the child
// objects interface, we bind the actual child to the first parameter. There are
// two reasons to do this: we save the need of creating WeakPtrFactories for all
// individual objects, and we ensure that is safe to destroy the object when
// handling the callback -- which is done by our OnApplicationUninstalled
// implementation.
void InstalledApplicationsRoot::OnUninstall(
    InstalledApplicationObject* installed_app_object,
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  if (!application_service_->Uninstall(installed_app_object->app_id())) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kInstalledApplicationDBusError,
            "Error trying to uninstall application with id "
            + installed_app_object->app_id());
    response_sender.Run(error_response.PassAs<dbus::Response>());
    return;
  }

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(response.Pass());
}

void InstalledApplicationsRoot::OnExported(
    const std::string& interface_name,
    const std::string& method_name,
    bool success) {
  if (!success) {
    LOG(WARNING) << "Error exporting method '" << interface_name
                 << "." << method_name << "' in '"
                 << kInstalledApplicationsRootPath.value() << "'.";
  }
}

}  // namespace application
}  // namespace xwalk
