// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/observer_list.h"
#include "content/public/browser/render_process_host_observer.h"
#include "ui/base/ui_base_types.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/security_policy.h"
#include "xwalk/runtime/browser/runtime.h"


namespace content {
class RenderProcessHost;
}

namespace xwalk {

class RuntimeContext;

namespace application {

class ApplicationHost;
class Manifest;
class SecurityPolicy;

// The Application class is representing an active (running) application.
// Application instances are owned by ApplicationService.
// ApplicationService will delete an Application instance when it is
// terminated.
// There's one-to-one correspondence between Application and Render Process
// Host, obtained from its "runtimes" (pages).
class Application : public Runtime::Observer,
                    public content::RenderProcessHostObserver {
 public:
  virtual ~Application();

  class Observer {
   public:
    // Invoked when application is terminated - all its pages (runtimes)
    // are closed.
    virtual void OnApplicationTerminated(Application* app) {}

   protected:
    virtual ~Observer() {}
  };

  struct LaunchParams {
    // Used only when running as service. Specifies the PID of the launcher
    // process.
    int32 launcher_pid;

    bool force_fullscreen;
    bool remote_debugging;
  };

  // Closes all the application's runtimes (application pages).
  // NOTE: Application is terminated asynchronously.
  // Please use ApplicationService::Observer::WillDestroyApplication()
  // interface to be notified about actual app termination.
  //
  // NOTE: ApplicationService deletes an Application instance
  // immediately after its termination.
  void Terminate();

  const std::set<Runtime*>& runtimes() const { return runtimes_; }

  // Returns the unique application id which is used to distinguish the
  // application amoung both running applications and installed ones
  // (ApplicationData objects).
  std::string id() const { return data_->ID(); }
  int GetRenderProcessHostID() const;
  content::RenderProcessHost* render_process_host() {
    return render_process_host_; }

  const ApplicationData* data() const { return data_.get(); }
  ApplicationData* data() { return data_.get(); }

  // Tells whether the application use the specified extension.
  bool UseExtension(const std::string& extension_name) const;

  // The runtime permission mapping is registered by extension which
  // implements some specific API, for example:
  // "bluetooth" -> "bluetooth.read, bluetooth.write, bluetooth.management"
  // Whenever there comes a API permission request, we can tell whether
  // this API is registered, if yes, return the according permission name.
  bool RegisterPermissions(const std::string& extension_name,
                           const std::string& perm_table);
  std::string GetRegisteredPermissionName(const std::string& extension_name,
                                          const std::string& api_name) const;

  StoredPermission GetPermission(PermissionType type,
                                 const std::string& permission_name) const;
  bool SetPermission(PermissionType type,
                     const std::string& permission_name,
                     StoredPermission perm);
  bool CanRequestURL(const GURL& url) const;

  void set_observer(Observer* observer) { observer_ = observer; }

  // FIXME(xinchao): This method will be deprecated soon.
  ui::WindowShowState window_show_state() const { return window_show_state_; }

 protected:
  Application(scoped_refptr<ApplicationData> data, RuntimeContext* context);
  virtual bool Launch(const LaunchParams& launch_params);
  virtual void InitSecurityPolicy();

  // Get the path of splash screen image. Return empty path by default.
  // Sub class can override it to return a specific path.
  virtual base::FilePath GetSplashScreenPath();

  std::set<Runtime*> runtimes_;
  RuntimeContext* runtime_context_;
  scoped_refptr<ApplicationData> const data_;
  // The application's render process host.
  content::RenderProcessHost* render_process_host_;
  content::WebContents* web_contents_;
  bool security_mode_enabled_;

  base::WeakPtr<Application> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 private:
  // We enforce ApplicationService ownership.
  friend class ApplicationService;
  static scoped_ptr<Application> Create(scoped_refptr<ApplicationData> data,
      RuntimeContext* context);
  // Runtime::Observer implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE;

  // content::RenderProcessHostObserver implementation.
  virtual void RenderProcessExited(content::RenderProcessHost* host,
                                   base::ProcessHandle handle,
                                   base::TerminationStatus status,
                                   int exit_code) OVERRIDE;
  virtual void RenderProcessHostDestroyed(
      content::RenderProcessHost* host) OVERRIDE;

  // Try to extract the URL from different possible keys for entry points in the
  // manifest, returns it and the entry point used.
  template <Manifest::Type> GURL GetStartURL();

  template <Manifest::Type>
  ui::WindowShowState GetWindowShowState(const LaunchParams& params);

  GURL GetAbsoluteURLFromKey(const std::string& key);

  void NotifyTermination();

  Observer* observer_;

  ui::WindowShowState window_show_state_;

  std::map<std::string, std::string> name_perm_map_;
  // Application's session permissions.
  StoredPermissionMap permission_map_;
  // Security policy.
  scoped_ptr<SecurityPolicy> security_policy_;
  // Remote debugging enabled or not for this Application
  bool remote_debugging_enabled_;
  // WeakPtrFactory should be always declared the last.
  base::WeakPtrFactory<Application> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_H_
