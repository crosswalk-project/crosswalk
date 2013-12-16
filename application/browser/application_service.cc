// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service.h"

#include <string>

#include "base/file_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/installer/package.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/application/browser/installer/tizen/package_installer.h"
#endif

using xwalk::RuntimeContext;

namespace {

void WaitForFinishLoad(content::WebContents* content) {
  class CloseAfterLoadObserver : public content::WebContentsObserver {
   public:
    static CloseAfterLoadObserver* Create(content::WebContents* content) {
      return new CloseAfterLoadObserver(content);
    }

    virtual void DidFinishLoad(
        int64 frame_id,
        const GURL& validate_url,
        bool is_main_frame,
        content::RenderViewHost* render_view_host) OVERRIDE {
      // FIXME: Quit message loop here at present. This should go away once
      // we have Application in place.
      base::MessageLoop::current()->QuitWhenIdle();
      delete this;
    }

   private:
    explicit CloseAfterLoadObserver(content::WebContents* content)
        : content::WebContentsObserver(content) {}
  };

  CloseAfterLoadObserver* observer = CloseAfterLoadObserver::Create(content);
}

#if defined(OS_TIZEN_MOBILE)
bool InstallPackageOnTizen(xwalk::application::ApplicationService* service,
                           const std::string& app_id,
                           const base::FilePath& data_dir) {
  scoped_ptr<xwalk::application::PackageInstaller> installer =
      xwalk::application::PackageInstaller::Create(service, app_id, data_dir);
  if (!installer || !installer->Install()) {
    LOG(ERROR) << "[ERR] An error occurred during installing on Tizen.";
    return false;
  }
  return true;
}

bool UninstallPackageOnTizen(xwalk::application::ApplicationService* service,
                             const std::string& app_id,
                             const base::FilePath& data_dir) {
  scoped_ptr<xwalk::application::PackageInstaller> installer =
      xwalk::application::PackageInstaller::Create(service, app_id, data_dir);
  if (!installer || !installer->Uninstall()) {
    LOG(ERROR) << "[ERR] An error occurred during uninstalling on Tizen.";
    return false;
  }
  return true;
}
#endif  // OS_TIZEN_MOBILE

}  // namespace

namespace xwalk {
namespace application {

const base::FilePath::CharType kApplicationsDir[] =
    FILE_PATH_LITERAL("applications");

ApplicationService::ApplicationService(RuntimeContext* runtime_context,
                                       ApplicationStorage* app_storage)
    : runtime_context_(runtime_context),
      app_storage_(app_storage) {
}

ApplicationService::~ApplicationService() {
}

bool ApplicationService::Install(const base::FilePath& path, std::string* id) {
  if (!base::PathExists(path))
    return false;

  const base::FilePath data_dir =
      runtime_context_->GetPath().Append(kApplicationsDir);

  // Make sure the kApplicationsDir exists under data_path, otherwise,
  // the installation will always fail because of moving application
  // resources into an invalid directory.
  if (!base::DirectoryExists(data_dir) &&
      !file_util::CreateDirectory(data_dir))
    return false;

  base::FilePath unpacked_dir;
  std::string app_id;
  if (!base::DirectoryExists(path)) {
    scoped_ptr<Package> package = Package::Create(path);
    if (package)
      app_id = package->Id();

    if (app_id.empty()) {
      LOG(ERROR) << "XPK/WGT file is invalid.";
      return false;
    }

    if (app_storage_->Contains(app_id)) {
      *id = app_id;
      LOG(INFO) << "Already installed: " << app_id;
      return false;
    }

    base::FilePath temp_dir;
    package->Extract(&temp_dir);
    unpacked_dir = data_dir.AppendASCII(app_id);
    if (base::DirectoryExists(unpacked_dir) &&
        !base::DeleteFile(unpacked_dir, true))
      return false;
    if (!base::Move(temp_dir, unpacked_dir))
      return false;
  } else {
    unpacked_dir = path;
  }

  std::string error;
  scoped_refptr<ApplicationData> application =
      LoadApplication(unpacked_dir,
                      app_id,
                      Manifest::COMMAND_LINE,
                      &error);
  if (!application) {
    LOG(ERROR) << "Error during application installation: " << error;
    return false;
  }

  if (!app_storage_->AddApplication(application)) {
    LOG(ERROR) << "Application with id " << application->ID()
               << " couldn't be installed.";
    return false;
  }

#if defined(OS_TIZEN_MOBILE)
  if (!InstallPackageOnTizen(this, application->ID(),
                             runtime_context_->GetPath()))
    return false;
#endif

  LOG(INFO) << "Installed application with id: " << application->ID()
            << " successfully.";
  *id = application->ID();

  FOR_EACH_OBSERVER(Observer, observers_,
                    OnApplicationInstalled(application->ID()));

  // We need to run main document after installation in order to
  // register system events.
  if (application->HasMainDocument() && Launch(application->ID())) {
    DCHECK(applications_.size() == 1);
    WaitForFinishLoad(applications_[0]->GetMainDocumentRuntime()->web_contents());
  }

  return true;
}

bool ApplicationService::Uninstall(const std::string& id) {
#if defined(OS_TIZEN_MOBILE)
  if (!UninstallPackageOnTizen(this, id, runtime_context_->GetPath()))
    return false;
#endif

  if (!app_storage_->RemoveApplication(id)) {
    LOG(ERROR) << "Cannot uninstall application with id " << id
               << "; application is not installed.";
    return false;
  }

  const base::FilePath resources =
      runtime_context_->GetPath().Append(kApplicationsDir).AppendASCII(id);
  if (base::DirectoryExists(resources) &&
      !base::DeleteFile(resources, true)) {
    LOG(ERROR) << "Error occurred while trying to remove application with id "
               << id << "; Cannot remove all resources.";
    return false;
  }

  FOR_EACH_OBSERVER(Observer, observers_, OnApplicationUninstalled(id));

  return true;
}

Application* ApplicationService::Launch(const std::string& id) {
  scoped_refptr<const ApplicationData> application_data =
          app_storage_->GetApplicationData(id);
  if (!application_data) {
    LOG(ERROR) << "Application with id " << id << " haven't installed.";
    return false;
  }

  return Launch(application_data);
}

Application* ApplicationService::Launch(const base::FilePath& path) {
  if (!base::DirectoryExists(path))
    return false;

  std::string error;
  scoped_refptr<const ApplicationData> application_data =
      LoadApplication(path, Manifest::COMMAND_LINE, &error);

  if (!application_data) {
    LOG(ERROR) << "Error during launch application: " << error;
    return false;
  }

  return Launch(application_data);
}

void ApplicationService::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ApplicationService::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void ApplicationService::OnApplicationTerminated(
                                      Application* application) OVERRIDE {
  ScopedVector<Application>::iterator found = std::find(
            applications_.begin(), applications_.end(), application);
  CHECK(found != applications_.end());
  applications_.erase(found);
}

Application* ApplicationService::Launch(
    scoped_refptr<const ApplicationData> application_data) {
  ApplicationSystem* system = runtime_context_->GetApplicationSystem();
  ApplicationEventManager* event_manager = system->event_manager();
  event_manager->OnAppLoaded(application_data->ID());

  scoped_ptr<Application> application(new Application(application_data,
                                                      runtime_context_, this));

  if (!application->Launch())
    return NULL;

  applications_.push_back(application.release());
  return applications_.back();
}

}  // namespace application
}  // namespace xwalk
