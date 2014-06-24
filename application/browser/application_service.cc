// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service.h"

#include <set>
#include <string>

#include "base/files/file_enumerator.h"
#include "base/file_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/installer/package.h"
#include "xwalk/application/browser/installer/package_installer.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_paths.h"

#if defined(OS_TIZEN)
#include "xwalk/application/browser/application_tizen.h"
#endif

namespace xwalk {
namespace application {

ApplicationService::ApplicationService(RuntimeContext* runtime_context,
                                       ApplicationStorage* app_storage)
    : runtime_context_(runtime_context),
      application_storage_(app_storage),
      package_installer_(PackageInstaller::Create(app_storage)) {
}

ApplicationService::~ApplicationService() {
}

bool ApplicationService::Install(const base::FilePath& path, std::string* id) {
  if (!package_installer_->Install(path, id))
    return false;

  FOR_EACH_OBSERVER(Observer, observers_,
                    OnApplicationInstalled(*id));

  return true;
}

bool ApplicationService::Update(const std::string& id,
                                const base::FilePath& path) {
  if (Application* app = GetApplicationByID(id)) {
    LOG(INFO) << "Try to terminate the running application before uninstall.";
    app->Terminate();
  }

  if (!package_installer_->Update(id, path))
    return false;

  FOR_EACH_OBSERVER(Observer, observers_,
                    OnApplicationUpdated(id));

  return true;
}

bool ApplicationService::Uninstall(const std::string& id) {
  if (Application* app = GetApplicationByID(id)) {
    LOG(INFO) << "Try to terminate the running application before uninstall.";
    app->Terminate();
  }

  if (!package_installer_->Uninstall(id))
    return false;

  FOR_EACH_OBSERVER(Observer, observers_, OnApplicationUninstalled(id));

  return true;
}

void ApplicationService::ChangeLocale(const std::string& locale) {
  ApplicationData::ApplicationDataMap apps;
  if (!application_storage_->GetInstalledApplications(apps))
    return;

  ApplicationData::ApplicationDataMap::const_iterator it;
  for (it = apps.begin(); it != apps.end(); ++it) {
    base::string16 error;
    std::string old_name = it->second->Name();
    if (!it->second->SetApplicationLocale(locale, &error)) {
      LOG(ERROR) << "Error when set locale " << locale
                 << " to application " << it->second->ID()
                 << "error : " << error;
    }
    if (old_name != it->second->Name()) {
      // After we has changed the application locale, we might get a new name in
      // the new locale, so call all observer for this event.
      FOR_EACH_OBSERVER(
          Observer, observers_,
          OnApplicationNameChanged(it->second->ID(), it->second->Name()));
    }
  }
}

Application* ApplicationService::Launch(
    scoped_refptr<ApplicationData> application_data,
    const Application::LaunchParams& launch_params) {
  if (GetApplicationByID(application_data->ID()) != NULL) {
    LOG(INFO) << "Application with id: " << application_data->ID()
              << " is already running.";
    // FIXME: we need to notify application that it had been attempted
    // to invoke and let the application to define the further behavior.
    return NULL;
  }

#if defined(OS_TIZEN)
  Application* application(new ApplicationTizen(application_data,
    runtime_context_, this));
#else
  Application* application(new Application(application_data,
    runtime_context_, this));
#endif

  ScopedVector<Application>::iterator app_iter =
      applications_.insert(applications_.end(), application);

  if (!application->Launch(launch_params)) {
    applications_.erase(app_iter);
    return NULL;
  }

  FOR_EACH_OBSERVER(Observer, observers_,
                    DidLaunchApplication(application));

  return application;
}

Application* ApplicationService::Launch(
    const std::string& id, const Application::LaunchParams& params) {
  Application* application = NULL;
  scoped_refptr<ApplicationData> application_data =
    application_storage_->GetApplicationData(id);
  if (!application_data) {
    LOG(ERROR) << "Application with id " << id << " is not installed.";
    return NULL;
  }

  return Launch(application_data, params);
}

Application* ApplicationService::Launch(
    const base::FilePath& path, const Application::LaunchParams& params) {
  Application* application = NULL;
  if (!base::DirectoryExists(path))
    return NULL;

  std::string error;
  scoped_refptr<ApplicationData> application_data =
      LoadApplication(path, Manifest::COMMAND_LINE, &error);

  if (!application_data) {
    LOG(ERROR) << "Error occurred while trying to launch application: "
               << error;
    return NULL;
  }

  return Launch(application_data, params);
}

namespace {

struct ApplicationRenderHostIDComparator {
    explicit ApplicationRenderHostIDComparator(int id) : id(id) { }
    bool operator() (Application* application) {
      return id == application->GetRenderProcessHostID();
    }
    int id;
};

struct ApplicationIDComparator {
    explicit ApplicationIDComparator(const std::string& app_id)
      : app_id(app_id) { }
    bool operator() (Application* application) {
      return app_id == application->id();
    }
    std::string app_id;
};

}  // namespace

Application* ApplicationService::GetApplicationByRenderHostID(int id) const {
  ApplicationRenderHostIDComparator comparator(id);
  ScopedVector<Application>::const_iterator found = std::find_if(
      applications_.begin(), applications_.end(), comparator);
  if (found != applications_.end())
    return *found;
  return NULL;
}

Application* ApplicationService::GetApplicationByID(
    const std::string& app_id) const {
  ApplicationIDComparator comparator(app_id);
  ScopedVector<Application>::const_iterator found = std::find_if(
      applications_.begin(), applications_.end(), comparator);
  if (found != applications_.end())
    return *found;
  return NULL;
}

void ApplicationService::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ApplicationService::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void ApplicationService::OnApplicationTerminated(
                                      Application* application) {
  ScopedVector<Application>::iterator found = std::find(
      applications_.begin(), applications_.end(), application);
  CHECK(found != applications_.end());
  FOR_EACH_OBSERVER(Observer, observers_,
                    WillDestroyApplication(application));
  applications_.erase(found);
  if (!XWalkRunner::GetInstance()->is_running_as_service() &&
      applications_.empty()) {
    base::MessageLoop::current()->PostTask(
            FROM_HERE, base::MessageLoop::QuitClosure());
  }
}

void ApplicationService::CheckAPIAccessControl(const std::string& app_id,
    const std::string& extension_name,
    const std::string& api_name, const PermissionCallback& callback) {
  Application* app = GetApplicationByID(app_id);
  if (!app) {
    LOG(ERROR) << "No running application found with ID: "
      << app_id;
    callback.Run(UNDEFINED_RUNTIME_PERM);
    return;
  }
  if (!app->UseExtension(extension_name)) {
    LOG(ERROR) << "Can not find extension: "
      << extension_name << " of Application with ID: "
      << app_id;
    callback.Run(UNDEFINED_RUNTIME_PERM);
    return;
  }
  // Permission name should have been registered at extension initialization.
  std::string permission_name =
      app->GetRegisteredPermissionName(extension_name, api_name);
  if (permission_name.empty()) {
    LOG(ERROR) << "API: " << api_name << " of extension: "
      << extension_name << " not registered!";
    callback.Run(UNDEFINED_RUNTIME_PERM);
    return;
  }
  // Okay, since we have the permission name, let's get down to the policies.
  // First, find out whether the permission is stored for the current session.
  StoredPermission perm = app->GetPermission(
      SESSION_PERMISSION, permission_name);
  if (perm != UNDEFINED_STORED_PERM) {
    // "PROMPT" should not be in the session storage.
    DCHECK(perm != PROMPT);
    if (perm == ALLOW) {
      callback.Run(ALLOW_SESSION);
      return;
    }
    if (perm == DENY) {
      callback.Run(DENY_SESSION);
      return;
    }
    NOTREACHED();
  }
  // Then, query the persistent policy storage.
  perm = app->GetPermission(PERSISTENT_PERMISSION, permission_name);
  // Permission not found in persistent permission table, normally this should
  // not happen because all the permission needed by the application should be
  // contained in its manifest, so it also means that the application is asking
  // for something wasn't allowed.
  if (perm == UNDEFINED_STORED_PERM) {
    callback.Run(UNDEFINED_RUNTIME_PERM);
    return;
  }
  if (perm == PROMPT) {
    // TODO(Bai): We needed to pop-up a dialog asking user to chose one from
    // either allow/deny for session/one shot/forever. Then, we need to update
    // the session and persistent policy accordingly.
    callback.Run(UNDEFINED_RUNTIME_PERM);
    return;
  }
  if (perm == ALLOW) {
    callback.Run(ALLOW_ALWAYS);
    return;
  }
  if (perm == DENY) {
    callback.Run(DENY_ALWAYS);
    return;
  }
  NOTREACHED();
}

bool ApplicationService::RegisterPermissions(const std::string& app_id,
    const std::string& extension_name,
    const std::string& perm_table) {
  Application* app = GetApplicationByID(app_id);
  if (!app) {
    LOG(ERROR) << "No running application found with ID: " << app_id;
    return false;
  }
  if (!app->UseExtension(extension_name)) {
    LOG(ERROR) << "Can not find extension: "
               << extension_name << " of Application with ID: "
               << app_id;
    return false;
  }
  return app->RegisterPermissions(extension_name, perm_table);
}

}  // namespace application
}  // namespace xwalk
