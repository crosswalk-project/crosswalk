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
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/installer/package.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/permission_policy_manager.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/application/browser/installer/tizen/package_installer.h"
#include "xwalk/application/browser/installer/tizen/service_package_installer.h"
#endif

using xwalk::RuntimeContext;

namespace {

void CloseMessageLoop() {
  // FIXME: Quit message loop here at present. This should go away once
  // we have Application in place.
  base::MessageLoop::current()->QuitWhenIdle();
}

void WaitForEventAndClose(
    const std::string& app_id, const std::string& event_name,
    xwalk::application::ApplicationEventManager* event_manager) {
  class CloseOnEventArrived : public xwalk::application::EventObserver {
   public:
    static CloseOnEventArrived* Create(const std::string& event_name,
        xwalk::application::ApplicationEventManager* event_manager) {
      return new CloseOnEventArrived(event_name, event_manager);
    }

    virtual void Observe(
        const std::string& app_id,
        scoped_refptr<xwalk::application::Event> event) OVERRIDE {
      DCHECK(xwalk::application::kOnJavaScriptEventAck == event->name());
      std::string ack_event_name;
      event->args()->GetString(0, &ack_event_name);
      if (ack_event_name != event_name_)
        return;
      CloseMessageLoop();
      delete this;
    }

   private:
    CloseOnEventArrived(
        const std::string& event_name,
        xwalk::application::ApplicationEventManager* event_manager)
        : xwalk::application::EventObserver(event_manager),
          event_name_(event_name) {}

    std::string event_name_;
  };

  DCHECK(event_manager);
  CloseOnEventArrived* observer =
      CloseOnEventArrived::Create(event_name, event_manager);
  event_manager->AttachObserver(app_id,
      xwalk::application::kOnJavaScriptEventAck, observer);
}

void WaitForFinishLoad(
    scoped_refptr<xwalk::application::ApplicationData> application,
    xwalk::application::ApplicationEventManager* event_manager,
    content::WebContents* contents) {
  class CloseAfterLoadObserver : public content::WebContentsObserver {
   public:
    CloseAfterLoadObserver(
        scoped_refptr<xwalk::application::ApplicationData> application,
        xwalk::application::ApplicationEventManager* event_manager,
        content::WebContents* contents)
        : content::WebContentsObserver(contents),
          application_(application),
          event_manager_(event_manager) {
      DCHECK(application_);
      DCHECK(event_manager_);
    }

    virtual void DidFinishLoad(
        int64 frame_id,
        const GURL& validate_url,
        bool is_main_frame,
        content::RenderViewHost* render_view_host) OVERRIDE {
      if (!IsEventHandlerRegistered(xwalk::application::kOnInstalled)) {
        CloseMessageLoop();
      } else {
        scoped_ptr<base::ListValue> event_args(new base::ListValue);
        scoped_refptr<xwalk::application::Event> event =
            xwalk::application::Event::CreateEvent(
                xwalk::application::kOnInstalled, event_args.Pass());
        event_manager_->SendEvent(application_->ID(), event);

        WaitForEventAndClose(
            application_->ID(), event->name(), event_manager_);
      }
      delete this;
    }

   private:
    bool IsEventHandlerRegistered(const std::string& event_name) const {
      const std::set<std::string>& events = application_->GetEvents();
      return events.find(event_name) != events.end();
    }

    scoped_refptr<xwalk::application::ApplicationData> application_;
    xwalk::application::ApplicationEventManager* event_manager_;
  };

  // This object is self-destroyed when an event occurs.
  new CloseAfterLoadObserver(application, event_manager, contents);
}

#if defined(OS_TIZEN_MOBILE)
bool InstallPackageOnTizen(xwalk::application::ApplicationService* service,
                           xwalk::application::ApplicationStorage* storage,
                           xwalk::application::ApplicationData* application,
                           const base::FilePath& data_dir) {
  if (xwalk::XWalkRunner::GetInstance()->is_running_as_service()) {
    return InstallApplicationForTizen(application, data_dir);
  }

  scoped_ptr<xwalk::application::PackageInstaller> installer =
      xwalk::application::PackageInstaller::Create(service, storage,
                                                   application->ID(), data_dir);
  if (!installer || !installer->Install()) {
    LOG(ERROR) << "An error occurred during installation on Tizen.";
    return false;
  }
  return true;
}

bool UninstallPackageOnTizen(xwalk::application::ApplicationService* service,
                             xwalk::application::ApplicationStorage* storage,
                             xwalk::application::ApplicationData* application,
                             const base::FilePath& data_dir) {
  if (xwalk::XWalkRunner::GetInstance()->is_running_as_service()) {
    return UninstallApplicationForTizen(application, data_dir);
  }

  scoped_ptr<xwalk::application::PackageInstaller> installer =
      xwalk::application::PackageInstaller::Create(service, storage,
                                                   application->ID(), data_dir);
  if (!installer || !installer->Uninstall()) {
    LOG(ERROR) << "An error occurred during uninstallation on Tizen.";
    return false;
  }
  return true;
}
#endif  // OS_TIZEN_MOBILE

bool CopyDirectoryContents(const base::FilePath& from,
    const base::FilePath& to) {
  base::FileEnumerator iter(from, false,
      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);
  for (base::FilePath path = iter.Next(); !path.empty(); path = iter.Next()) {
    if (iter.GetInfo().IsDirectory()) {
      if (!base::CopyDirectory(path, to, true))
        return false;
    } else if (!base::CopyFile(path, to.Append(path.BaseName()))) {
        return false;
    }
  }

  return true;
}

}  // namespace

namespace xwalk {
namespace application {

const base::FilePath::CharType kApplicationsDir[] =
    FILE_PATH_LITERAL("applications");

ApplicationService::ApplicationService(RuntimeContext* runtime_context,
                                       ApplicationStorage* app_storage,
                                       ApplicationEventManager* event_manager)
    : runtime_context_(runtime_context),
      application_storage_(app_storage),
      event_manager_(event_manager),
      permission_policy_manager_(new PermissionPolicyManager()) {
  AddObserver(event_manager);
}

ApplicationService::~ApplicationService() {
}

bool ApplicationService::Install(const base::FilePath& path, std::string* id) {
  // FIXME(leandro): Installation is not robust enough -- should any step
  // fail, it can't roll back to a consistent state.
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
  scoped_ptr<Package> package;
  if (!base::DirectoryExists(path)) {
    package = Package::Create(path);
    package->Extract(&unpacked_dir);
  } else {
    unpacked_dir = path;
  }

  std::string error;
  scoped_refptr<ApplicationData> application_data = LoadApplication(
                      unpacked_dir, Manifest::COMMAND_LINE, &error);
  if (!application_data) {
    LOG(ERROR) << "Error during application installation: " << error;
    return false;
  }

  if (application_storage_->Contains(application_data->ID())) {
    *id = application_data->ID();
    LOG(INFO) << "Already installed: " << id;
    return false;
  }

  base::FilePath app_dir = data_dir.AppendASCII(application_data->ID());
  if (base::DirectoryExists(app_dir)) {
    if (!base::DeleteFile(app_dir, true))
      return false;
  }
  if (!package) {
    if (!file_util::CreateDirectory(app_dir))
      return false;
    if (!CopyDirectoryContents(unpacked_dir, app_dir))
      return false;
  } else {
    if (!base::Move(unpacked_dir, app_dir))
      return false;
  }

  application_data->SetPath(app_dir);

  if (!application_storage_->AddApplication(application_data)) {
    LOG(ERROR) << "Application with id " << application_data->ID()
               << " couldn't be installed.";
    return false;
  }

#if defined(OS_TIZEN_MOBILE)
  if (!InstallPackageOnTizen(this, application_storage_,
                             application_data.get(),
                             runtime_context_->GetPath())) {
    application_storage_->RemoveApplication(application_data->ID());
    return false;
  }
#endif

  LOG(INFO) << "Application be installed in: " << app_dir.MaybeAsASCII();
  LOG(INFO) << "Installed application with id: " << application_data->ID()
            << " successfully.";
  *id = application_data->ID();

  FOR_EACH_OBSERVER(Observer, observers_,
                    OnApplicationInstalled(application_data->ID()));

  // We need to run main document after installation in order to
  // register system events.
  if (application_data->HasMainDocument()) {
    if (Application* application = Launch(application_data->ID())) {
      WaitForFinishLoad(application->data(), event_manager_,
          application->GetMainDocumentRuntime()->web_contents());
    }
  }

  return true;
}

bool ApplicationService::Uninstall(const std::string& id) {
  bool result = true;

  scoped_refptr<ApplicationData> application =
      application_storage_->GetApplicationData(id);
  if (!application) {
    LOG(ERROR) << "Cannot uninstall application with id " << id
               << "; invalid application id";
    return false;
  }

#if defined(OS_TIZEN_MOBILE)
  if (!UninstallPackageOnTizen(this, application_storage_, application.get(),
                               runtime_context_->GetPath()))
    result = false;
#endif

  if (!application_storage_->RemoveApplication(id)) {
    LOG(ERROR) << "Cannot uninstall application with id " << id
               << "; application is not installed.";
    result = false;
  }

  const base::FilePath resources =
      runtime_context_->GetPath().Append(kApplicationsDir).AppendASCII(id);
  if (base::DirectoryExists(resources) &&
      !base::DeleteFile(resources, true)) {
    LOG(ERROR) << "Error occurred while trying to remove application with id "
               << id << "; Cannot remove all resources.";
    result = false;
  }

  FOR_EACH_OBSERVER(Observer, observers_, OnApplicationUninstalled(id));

  return result;
}

Application* ApplicationService::Launch(const std::string& id) {
  scoped_refptr<ApplicationData> application_data =
    application_storage_->GetApplicationData(id);
  if (!application_data) {
    LOG(ERROR) << "Application with id " << id << " is not installed.";
    return NULL;
  }

  return Launch(application_data, Application::LaunchParams());
}

Application* ApplicationService::Launch(const base::FilePath& path) {
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

  return Launch(application_data, Application::LaunchParams());
}

Application* ApplicationService::Launch(const GURL& url) {
  namespace keys = xwalk::application_manifest_keys;

  const std::string& url_spec = url.spec();
  DCHECK(!url_spec.empty());
  const std::string& app_id = GenerateId(url_spec);
  // FIXME: we need to handle hash collisions.
  DCHECK(!application_storage_->GetApplicationData(app_id));

  base::DictionaryValue manifest;
  // FIXME: define permissions!
  manifest.SetString(keys::kURLKey, url_spec);
  manifest.SetString(keys::kNameKey, "XWalk Browser");
  manifest.SetString(keys::kVersionKey, "0");
  manifest.SetInteger(keys::kManifestVersionKey, 1);
  std::string error;
  scoped_refptr<ApplicationData> application_data = ApplicationData::Create(
            base::FilePath(), Manifest::COMMAND_LINE, manifest, app_id, &error);
  if (!application_data) {
    LOG(ERROR) << "Error occurred while trying to launch application: "
               << error;
    return NULL;
  }

  Application::LaunchParams launch_params;
  launch_params.entry_points = Application::URLKey;
  return Launch(application_data, launch_params);
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
  if (applications_.empty()) {
    base::MessageLoop::current()->PostTask(
            FROM_HERE, base::MessageLoop::QuitClosure());
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

  event_manager_->AddEventRouterForApp(application_data);
  Application* application(new Application(application_data,
                                           runtime_context_,
                                           this));
  ScopedVector<Application>::iterator app_iter =
      applications_.insert(applications_.end(), application);

  if (!application->Launch(launch_params)) {
    event_manager_->RemoveEventRouterForApp(application_data);
    applications_.erase(app_iter);
    return NULL;
  }

  FOR_EACH_OBSERVER(Observer, observers_,
                    DidLaunchApplication(application));

  return application;
}

}  // namespace application
}  // namespace xwalk
