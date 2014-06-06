// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/linux/installed_applications_manager.h"

#include <string>
#include "base/bind.h"
#include "base/files/file_path.h"
#include "content/public/browser/browser_thread.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/linux/installed_application_object.h"

namespace {

// D-Bus Interface implemented by the manager object of installed applications.
//
// Methods:
//
//   Install(string path) -> ObjectPath
//     Will install application at "path", that should be an absolute path to
//     the package file. If installation is successful, returns the ObjectPath
//     of the InstalledApplication object that represents it.
const char kInstalledManagerDBusInterface[] =
    "org.crosswalkproject.Installed.Manager1";

const char kInstalledManagerDBusError[] =
    "org.crosswalkproject.Installed.Manager.Error";

const dbus::ObjectPath kInstalledManagerDBusPath("/installed1");

dbus::ObjectPath GetInstalledPathForAppID(const std::string& app_id) {
  return dbus::ObjectPath(kInstalledManagerDBusPath.value() + "/" + app_id);
}

}  // namespace

namespace xwalk {
namespace application {

InstalledApplicationsManager::InstalledApplicationsManager(
    scoped_refptr<dbus::Bus> bus, ApplicationService* service,
    ApplicationStorage* app_storage)
    : weak_factory_(this),
      application_service_(service),
      app_storage_(app_storage),
      adaptor_(bus, kInstalledManagerDBusPath) {
  application_service_->AddObserver(this);

  adaptor_.manager_object()->ExportMethod(
      kInstalledManagerDBusInterface, "Install",
      base::Bind(&InstalledApplicationsManager::OnInstall,
                 weak_factory_.GetWeakPtr()),
      base::Bind(&InstalledApplicationsManager::OnExported,
                 weak_factory_.GetWeakPtr()));

  AddInitialObjects();
}

InstalledApplicationsManager::~InstalledApplicationsManager() {
  application_service_->RemoveObserver(this);
}

void InstalledApplicationsManager::OnApplicationInstalled(
    const std::string& app_id) {
  AddObject(app_storage_->GetApplicationData(app_id));
}

void InstalledApplicationsManager::OnApplicationUninstalled(
    const std::string& app_id) {
  adaptor_.RemoveManagedObject(GetInstalledPathForAppID(app_id));
}

void InstalledApplicationsManager::OnApplicationUpdated(
    const std::string& app_id) {
  if (scoped_refptr<ApplicationData> app =
      app_storage_->GetApplicationData(app_id))
    OnApplicationNameChanged(app_id, app->Name());
}

void InstalledApplicationsManager::OnApplicationNameChanged(
    const std::string& app_id, const std::string& app_name) {
  InstalledApplicationObject* object =
      static_cast<InstalledApplicationObject*>(
          adaptor_.GetManagedObject(GetInstalledPathForAppID(app_id)));
  object->OnApplicationNameChanged(app_name);
}

void InstalledApplicationsManager::AddInitialObjects() {
  const ApplicationData::ApplicationDataMap& apps =
      app_storage_->GetInstalledApplications();
  ApplicationData::ApplicationDataMap::const_iterator it;
  for (it = apps.begin(); it != apps.end(); ++it)
    AddObject(it->second);
}

void InstalledApplicationsManager::AddObject(
    scoped_refptr<const ApplicationData> app) {
  scoped_ptr<InstalledApplicationObject> object(
      new InstalledApplicationObject(
          adaptor_.bus(), GetInstalledPathForAppID(app->ID()), app));

  // See comment in InstalledApplicationsManager::OnUninstall().
  object->ExportUninstallMethod(
      base::Bind(&InstalledApplicationsManager::OnUninstall,
                 weak_factory_.GetWeakPtr(),
                 base::Unretained(object.get())),
      base::Bind(&InstalledApplicationsManager::OnExported,
                 weak_factory_.GetWeakPtr()));

  adaptor_.AddManagedObject(object.PassAs<dbus::ManagedObject>());
}

namespace {

scoped_ptr<dbus::Response> CreateError(dbus::MethodCall* method_call,
                                       const std::string& message) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kInstalledManagerDBusError, message);
    return error_response.PassAs<dbus::Response>();
}

scoped_ptr<dbus::Response> doInstall(dbus::MethodCall* method_call,
                                     ApplicationService* service,
                                     const base::FilePath file_path,
                                     dbus::ObjectManagerAdaptor* adaptor) {
  std::string app_id;
  if (!service->Install(file_path, &app_id) &&
      (app_id.empty() || !service->Update(app_id, file_path))) {
    scoped_ptr<dbus::Response> response =
        CreateError(method_call,
                    "Error installing/updating application with path: "
                    + file_path.value());
    return response.Pass();
  }

  dbus::ManagedObject* managed_object =
      adaptor->GetManagedObject(GetInstalledPathForAppID(app_id));
  CHECK(managed_object);

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendObjectPath(managed_object->path());
  return response.Pass();
}

scoped_ptr<dbus::Response> doUninstall(dbus::MethodCall* method_call,
                                       ApplicationService* service,
                                       const std::string app_id) {
  if (!service->Uninstall(app_id)) {
    scoped_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kInstalledApplicationDBusError,
            "Error trying to uninstall application with id "
            + app_id);
    return error_response.PassAs<dbus::Response>();
  }

  scoped_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  return response.Pass();
}

}  // namespace

void InstalledApplicationsManager::OnInstall(
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

  content::BrowserThread::PostTaskAndReplyWithResult(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&doInstall,
                 base::Unretained(method_call),
                 base::Unretained(application_service_),
                 file_path,
                 base::Unretained(&adaptor_)),
      response_sender);
}

// InstalledApplicationsManager implements the callback exposed in the child
// objects interface, we bind the actual child to the first parameter. There are
// two reasons to do this: we save the need of creating WeakPtrFactories for all
// individual objects, and we ensure that is safe to destroy the object when
// handling the callback -- which is done by our OnApplicationUninstalled
// implementation.
void InstalledApplicationsManager::OnUninstall(
    InstalledApplicationObject* installed_app_object,
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  content::BrowserThread::PostTaskAndReplyWithResult(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&doUninstall,
                 base::Unretained(method_call),
                 base::Unretained(application_service_),
                 installed_app_object->app_id()),
      response_sender);
}

void InstalledApplicationsManager::OnExported(
    const std::string& interface_name,
    const std::string& method_name,
    bool success) {
  if (!success) {
    LOG(WARNING) << "Error exporting method '" << interface_name
                 << "." << method_name << "' in '"
                 << kInstalledManagerDBusPath.value() << "'.";
  }
}

}  // namespace application
}  // namespace xwalk
