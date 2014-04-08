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
#include "base/version.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/views/widget/widget.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/installer/package.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/application/common/permission_policy_manager.h"
#include "xwalk/application/common/permission_dialog.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_paths.h"

#if defined(OS_TIZEN)
#include "xwalk/application/browser/installer/tizen/service_package_installer.h"
#endif

using content::BrowserThread;
namespace xwalk {
namespace application {

namespace {

void CloseMessageLoop() {
  // FIXME: Quit message loop here at present. This should go away once
  // we have Application in place.
  base::MessageLoop::current()->QuitWhenIdle();
}

void WaitForEventAndClose(
    const std::string& app_id, const std::string& event_name,
    ApplicationEventManager* event_manager) {
  class CloseOnEventArrived : public EventObserver {
   public:
    static CloseOnEventArrived* Create(const std::string& event_name,
        ApplicationEventManager* event_manager) {
      return new CloseOnEventArrived(event_name, event_manager);
    }

    virtual void Observe(
        const std::string& app_id,
        scoped_refptr<Event> event) OVERRIDE {
      DCHECK(kOnJavaScriptEventAck == event->name());
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
        ApplicationEventManager* event_manager)
        : EventObserver(event_manager),
          event_name_(event_name) {}

    std::string event_name_;
  };

  DCHECK(event_manager);
  CloseOnEventArrived* observer =
      CloseOnEventArrived::Create(event_name, event_manager);
  event_manager->AttachObserver(app_id,
      kOnJavaScriptEventAck, observer);
}

void WaitForFinishLoad(
    scoped_refptr<ApplicationData> application,
    ApplicationEventManager* event_manager,
    content::WebContents* contents) {
  class CloseAfterLoadObserver : public content::WebContentsObserver {
   public:
    CloseAfterLoadObserver(
        scoped_refptr<ApplicationData> application,
        ApplicationEventManager* event_manager,
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
      if (!IsEventHandlerRegistered(kOnInstalled)) {
        CloseMessageLoop();
      } else {
        scoped_ptr<base::ListValue> event_args(new base::ListValue);
        scoped_refptr<Event> event =
            Event::CreateEvent(
                kOnInstalled, event_args.Pass());
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

    scoped_refptr<ApplicationData> application_;
    ApplicationEventManager* event_manager_;
  };

  // This object is self-destroyed when an event occurs.
  new CloseAfterLoadObserver(application, event_manager, contents);
}

void SaveSystemEventsInfo(
    ApplicationService* application_service,
    scoped_refptr<ApplicationData> application_data,
    ApplicationEventManager* event_manager) {
  // We need to run main document after installation in order to
  // register system events.
  if (application_data->HasMainDocument()) {
    if (Application* application =
        application_service->Launch(application_data->ID())) {
      WaitForFinishLoad(application->data(), event_manager,
                        application->GetMainDocumentRuntime()->web_contents());
    }
  }
}

#if defined(OS_TIZEN)
bool InstallPackageOnTizen(ApplicationData* application_data,
                           const base::FilePath& data_dir) {
  if (!XWalkRunner::GetInstance()->is_running_as_service()) {
    LOG(ERROR) << "Installation on Tizen is only possible in"
               << "service mode via 'xwalkctl' utility.";
    return false;
  }

  return InstallApplicationForTizen(application_data, data_dir);
}

bool UninstallPackageOnTizen(ApplicationData* application_data,
                             const base::FilePath& data_dir) {
  if (!XWalkRunner::GetInstance()->is_running_as_service()) {
    LOG(ERROR) << "Uninstallation on Tizen is only possible in"
               << "service mode using 'xwalkctl' utility.";
    return false;
  }

  return UninstallApplicationForTizen(application_data, data_dir);
}
#endif  // OS_TIZEN

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

void RemoveWidgetStorageFiles(const base::FilePath& storage_path,
                              const std::string& app_id) {
  base::FileEnumerator iter(storage_path, true,
                            base::FileEnumerator::FILES);
  for (base::FilePath file = iter.Next(); !file.empty(); file = iter.Next()) {
    if (file.MaybeAsASCII().find(app_id) != std::string::npos)
      base::DeleteFile(file, false);
  }
}

}  // namespace

const base::FilePath::CharType kApplicationsDir[] =
    FILE_PATH_LITERAL("applications");

ApplicationService::ApplicationService(RuntimeContext* runtime_context,
                                       ApplicationStorage* app_storage,
                                       ApplicationEventManager* event_manager)
    : runtime_context_(runtime_context),
      application_storage_(app_storage),
      event_manager_(event_manager),
      permission_policy_handler_(new PermissionPolicyManager()) {
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
      !base::CreateDirectory(data_dir))
    return false;

  std::string app_id;
  base::FilePath unpacked_dir;
  scoped_ptr<Package> package;
  if (!base::DirectoryExists(path)) {
    package = Package::Create(path);
    package->Extract(&unpacked_dir);
    app_id = package->Id();
  } else {
    unpacked_dir = path;
  }

  std::string error;
  scoped_refptr<ApplicationData> application_data = LoadApplication(
      unpacked_dir, app_id, Manifest::COMMAND_LINE, &error);
  if (!application_data) {
    LOG(ERROR) << "Error during application installation: " << error;
    return false;
  }
  if (!permission_policy_handler_->
      InitApplicationPermission(application_data)) {
    LOG(ERROR) << "Application permission data is invalid";
    return false;
  }

  if (application_storage_->Contains(application_data->ID())) {
    *id = application_data->ID();
    LOG(INFO) << "Already installed: " << *id;
    return false;
  }

  base::FilePath app_dir = data_dir.AppendASCII(application_data->ID());
  if (base::DirectoryExists(app_dir)) {
    if (!base::DeleteFile(app_dir, true))
      return false;
  }
  if (!package) {
    if (!base::CreateDirectory(app_dir))
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

#if defined(OS_TIZEN)
  if (!InstallPackageOnTizen(application_data,
                             runtime_context_->GetPath())) {
    application_storage_->RemoveApplication(application_data->ID());
    return false;
  }
#endif

  LOG(INFO) << "Application be installed in: " << app_dir.MaybeAsASCII();
  LOG(INFO) << "Installed application with id: " << application_data->ID()
            << " successfully.";
  *id = application_data->ID();

  SaveSystemEventsInfo(this, application_data, event_manager_);

  FOR_EACH_OBSERVER(Observer, observers_,
                    OnApplicationInstalled(application_data->ID()));

  return true;
}

bool ApplicationService::Update(const std::string& id,
                                const base::FilePath& path) {
  if (!base::PathExists(path)) {
    LOG(ERROR) << "The XPK/WGT package file " << path.value() << " is invalid.";
    return false;
  }

  if (base::DirectoryExists(path)) {
    LOG(WARNING) << "Can not update an unpacked XPK/WGT package.";
    return false;
  }

  base::FilePath unpacked_dir;
  base::FilePath origin_dir;
  std::string app_id;
  scoped_ptr<Package> package = Package::Create(path);
  if (!package) {
    LOG(ERROR) << "XPK/WGT file is invalid.";
    return false;
  }

  app_id = package->Id();

  if (app_id.empty()) {
    LOG(ERROR) << "XPK/WGT file is invalid, and the application id is empty.";
    return false;
  }

  if (id.empty() ||
      id.compare(app_id) != 0) {
    LOG(ERROR) << "The XPK/WGT file is not the same as expecting.";
    return false;
  }

  if (!package->Extract(&unpacked_dir))
    return false;

  std::string error;
  scoped_refptr<ApplicationData> new_application =
      LoadApplication(unpacked_dir,
                      app_id,
                      Manifest::COMMAND_LINE,
                      &error);
  if (!new_application) {
    LOG(ERROR) << "An error occurred during application updating: " << error;
    return false;
  }

  scoped_refptr<ApplicationData> old_application =
      application_storage_->GetApplicationData(app_id);
  if (!old_application) {
    LOG(INFO) << "Application haven't installed yet: " << app_id;
    return false;
  }

  if (old_application->Version()->CompareTo(
          *(new_application->Version())) >= 0) {
    LOG(INFO) << "The version number of new XPK/WGT package "
                 "should be higher than "
              << old_application->VersionString();
    return false;
  }

  const base::FilePath& app_dir = old_application->Path();
  const base::FilePath tmp_dir(app_dir.value()
                               + FILE_PATH_LITERAL(".tmp"));

  if (Application* app = GetApplicationByID(app_id)) {
    LOG(INFO) << "Try to terminate the running application before update.";
    app->Terminate(Application::Immediate);
  }

  if (!base::Move(app_dir, tmp_dir) ||
      !base::Move(unpacked_dir, app_dir))
    return false;

  new_application = LoadApplication(app_dir,
                                    app_id,
                                    Manifest::COMMAND_LINE,
                                    &error);
  if (!new_application) {
    LOG(ERROR) << "Error during loading new package: " << error;
    base::DeleteFile(app_dir, true);
    base::Move(tmp_dir, app_dir);
    return false;
  }

#if defined(OS_TIZEN)
  if (!UninstallPackageOnTizen(old_application,
                               runtime_context_->GetPath())) {
    base::DeleteFile(app_dir, true);
    base::Move(tmp_dir, app_dir);
    return false;
  }
#endif

  if (!application_storage_->UpdateApplication(new_application)) {
    LOG(ERROR) << "An Error occurred when updating the application.";
    base::DeleteFile(app_dir, true);
    base::Move(tmp_dir, app_dir);
#if defined(OS_TIZEN)
    InstallPackageOnTizen(old_application,
                          runtime_context_->GetPath());
#endif
    return false;
  }
#if defined(OS_TIZEN)
  if (!InstallPackageOnTizen(new_application,
                             runtime_context_->GetPath()))
    return false;
#endif
  base::DeleteFile(tmp_dir, true);

  SaveSystemEventsInfo(this, new_application, event_manager_);

  FOR_EACH_OBSERVER(Observer, observers_,
                    OnApplicationUpdated(app_id));

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

  if (Application* app = GetApplicationByID(id)) {
    LOG(INFO) << "Try to terminate the running application before uninstall.";
    app->Terminate(Application::Immediate);
  }

#if defined(OS_TIZEN)
  if (!UninstallPackageOnTizen(application,
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

  content::StoragePartition* partition =
      content::BrowserContext::GetStoragePartitionForSite(
          runtime_context_, application->GetBaseURLFromApplicationId(id));
  partition->ClearDataForOrigin(
      content::StoragePartition::REMOVE_DATA_MASK_ALL,
      content::StoragePartition::QUOTA_MANAGED_STORAGE_MASK_ALL,
      application->URL(),
      partition->GetURLRequestContext());

  base::FilePath path;
  PathService::Get(xwalk::DIR_WGT_STORAGE_PATH, &path);
  RemoveWidgetStorageFiles(path, id);

  FOR_EACH_OBSERVER(Observer, observers_, OnApplicationUninstalled(id));

  return result;
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

Application* ApplicationService::Launch(
    const std::string& id, const Application::LaunchParams& params) {
  Application* application = NULL;
  scoped_refptr<ApplicationData> application_data =
    application_storage_->GetApplicationData(id);
  if (!application_data) {
    LOG(ERROR) << "Application with id " << id << " is not installed.";
    return NULL;
  }

  if ((application = Launch(application_data, params))) {
    scoped_refptr<Event> event = Event::CreateEvent(
        kOnLaunched, scoped_ptr<base::ListValue>(new base::ListValue));
    event_manager_->SendEvent(application->id(), event);
  }
  return application;
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

  if ((application = Launch(application_data, params))) {
    scoped_refptr<Event> event = Event::CreateEvent(
        kOnLaunched, scoped_ptr<base::ListValue>(new base::ListValue));
    event_manager_->SendEvent(application->id(), event);
  }
  return application;
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
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
        base::Bind(&ApplicationService::ShowPermissionDialog,
        base::Unretained(this), app_id, permission_name, callback));
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

void ApplicationService::ShowPermissionDialog(const std::string& app_id,
    const std::string& permission,
    const PermissionCallback& callback) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  PermissionDialog* dialog_ = new PermissionDialog(app_id, permission);
  dialog_->SetShowDialogCallback(callback);
  views::Widget::CreateWindow(dialog_)->Show();
}

}  // namespace application
}  // namespace xwalk
