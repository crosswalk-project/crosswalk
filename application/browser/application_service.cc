// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service.h"

#if defined(OS_MACOSX)
#include <ext/hash_set>
#else
#include <hash_set>
#endif
#include <set>
#include <string>
#include <vector>

#include "base/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/common/xwalk_paths.h"

#if defined(OS_TIZEN)
#include "xwalk/application/browser/application_service_tizen.h"
#endif

namespace xwalk {

namespace application {

ApplicationService::ApplicationService(RuntimeContext* runtime_context)
  : runtime_context_(runtime_context) {
}

scoped_ptr<ApplicationService> ApplicationService::Create(
    RuntimeContext* runtime_context) {
#if defined(OS_TIZEN)
  return make_scoped_ptr<ApplicationService>(
    new ApplicationServiceTizen(runtime_context));
#else
  return make_scoped_ptr(new ApplicationService(runtime_context));
#endif
}

ApplicationService::~ApplicationService() {
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

  Application* application = Application::Create(application_data,
    runtime_context_).release();
  ScopedVector<Application>::iterator app_iter =
      applications_.insert(applications_.end(), application);

  if (!application->Launch(launch_params)) {
    applications_.erase(app_iter);
    return NULL;
  }

  application->set_observer(this);

  FOR_EACH_OBSERVER(Observer, observers_,
                    DidLaunchApplication(application));

  return application;
}

Application* ApplicationService::LaunchFromManifestPath(
    const base::FilePath& path, Manifest::Type manifest_type,
        const Application::LaunchParams& params) {
  std::string error;
  scoped_ptr<Manifest> manifest = LoadManifest(path, manifest_type, &error);
  if (!manifest) {
    LOG(ERROR) << "Failed to load manifest.";
    return NULL;
  }

  base::FilePath app_path = path.DirName();
  LOG(ERROR) << "Loading app from " << app_path.MaybeAsASCII();

  scoped_refptr<ApplicationData> application_data = ApplicationData::Create(
      app_path, std::string(), ApplicationData::LOCAL_DIRECTORY,
      manifest.Pass(), &error);
  if (!application_data.get()) {
    LOG(ERROR) << "Error occurred while trying to load application: "
               << error;
    return NULL;
  }

  return Launch(application_data, params);
}

Application* ApplicationService::LaunchFromPackagePath(
    const base::FilePath& path, const Application::LaunchParams& params) {
  scoped_ptr<Package> package = Package::Create(path);
  if (!package || !package->IsValid()) {
    LOG(ERROR) << "Failed to obtain valid package from "
               << path.AsUTF8Unsafe();
    return NULL;
  }

  base::FilePath tmp_dir, target_dir;
  if (!GetTempDir(&tmp_dir)) {
    LOG(ERROR) << "Failed to obtain system temp directory.";
    return NULL;
  }

#if defined (OS_WIN)
  base::CreateTemporaryDirInDir(tmp_dir,
      base::UTF8ToWide(package->name()), &target_dir);
#else
  base::CreateTemporaryDirInDir(tmp_dir, package->name(), &target_dir);
#endif
  if (!package->ExtractTo(target_dir)) {
    LOG(ERROR) << "Failed to unpack to a temporary directory: "
               << target_dir.MaybeAsASCII();
    return NULL;
  }

  std::string error;
  scoped_refptr<ApplicationData> application_data = LoadApplication(
      target_dir, std::string(), ApplicationData::TEMP_DIRECTORY,
      package->manifest_type(), &error);
  if (!application_data.get()) {
    LOG(ERROR) << "Error occurred while trying to load application: "
               << error;
    return NULL;
  }

  return Launch(application_data, params);
}

// Launch an application created from arbitrary url.
// FIXME: This application should have the same strict permissions
// as common browser apps.
Application* ApplicationService::LaunchHostedURL(
    const GURL& url, const Application::LaunchParams& params) {
  const std::string& url_spec = url.spec();
  if (url_spec.empty()) {
      LOG(ERROR) << "Failed to launch application from the URL: " << url;
      return NULL;
  }

  const std::string& app_id = GenerateId(url_spec);

  scoped_ptr<base::DictionaryValue> settings(new base::DictionaryValue());
  // FIXME: define permissions!
  settings->SetString(application_manifest_keys::kStartURLKey, url_spec);
  // FIXME: Why use URL as name?
  settings->SetString(application_manifest_keys::kNameKey, url_spec);
  settings->SetString(application_manifest_keys::kXWalkVersionKey, "0");

  scoped_ptr<Manifest> manifest(
      new Manifest(settings.Pass(), Manifest::TYPE_MANIFEST));

  std::string error;
  scoped_refptr<ApplicationData> app_data =
        ApplicationData::Create(base::FilePath(), app_id,
        ApplicationData::EXTERNAL_URL, manifest.Pass(), &error);
  DCHECK(app_data.get());

  return Launch(app_data, params);
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
  scoped_refptr<ApplicationData> app_data = application->data();
  applications_.erase(found);

  if (app_data->source_type() == ApplicationData::TEMP_DIRECTORY) {
      LOG(INFO) << "Deleting the app temporary directory "
                << app_data->path().AsUTF8Unsafe();
      content::BrowserThread::PostTask(content::BrowserThread::FILE,
          FROM_HERE, base::Bind(base::IgnoreResult(&base::DeleteFile),
                                app_data->path(), true /*recursive*/));
      // FIXME: So far we simply clean up all the app persistent data,
      // further we need to add an appropriate logic to handle it.
      content::BrowserContext::GarbageCollectStoragePartitions(
          runtime_context_,
          make_scoped_ptr(new base::hash_set<base::FilePath>()),
          base::Bind(&base::DoNothing));
  }

#if !defined(SHARED_PROCESS_MODE)
  if (applications_.empty()) {
    base::MessageLoop::current()->PostTask(
            FROM_HERE, base::MessageLoop::QuitClosure());
  }
#endif
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
