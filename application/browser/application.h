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
#include "xwalk/application/browser/application_security_policy.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/runtime/browser/runtime.h"

namespace content {
class RenderProcessHost;
}

namespace xwalk {

class XWalkBrowserContext;

namespace application {

class ApplicationHost;
class Manifest;
class ApplicationSecurityPolicy;

// The Application class is representing an active (running) application.
// Application instances are owned by ApplicationService.
// ApplicationService will delete an Application instance when it is
// terminated.
// There's one-to-one correspondence between Application and Render Process
// Host, obtained from its "runtimes" (pages).
class Application : public Runtime::Observer,
                    public content::RenderProcessHostObserver {
 public:
  ~Application() override;

  class Observer {
   public:
    // Invoked when application is terminated - all its pages (runtimes)
    // are closed.
    virtual void OnApplicationTerminated(Application* app) {}

   protected:
    virtual ~Observer() {}
  };

  // Closes all the application's runtimes (application pages).
  // NOTE: Application is terminated asynchronously.
  // Please use ApplicationService::Observer::WillDestroyApplication()
  // interface to be notified about actual app termination.
  //
  // NOTE: ApplicationService deletes an Application instance
  // immediately after its termination.
  void Terminate();

  const std::vector<Runtime*>& runtimes() const { return runtimes_.get(); }

  // Returns the unique application id which is used to distinguish the
  // application amoung both running applications and installed ones
  // (ApplicationData objects).
  std::string id() const { return data_->ID(); }
  int GetRenderProcessHostID() const;
  content::RenderProcessHost* render_process_host() {
    return render_process_host_; }

  const ApplicationSecurityPolicy* security_policy() const {
    return security_policy_.get();
  }

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

  base::WeakPtr<Application> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 protected:
  Application(scoped_refptr<ApplicationData> data,
              XWalkBrowserContext* context);
  virtual bool Launch();

  // Runtime::Observer implementation.
  void OnNewRuntimeAdded(Runtime* runtime) override;
  void OnRuntimeClosed(Runtime* runtime) override;
  void OnApplicationExitRequested(Runtime* runtime) override;

  // Get the path of splash screen image. Return empty path by default.
  // Sub class can override it to return a specific path.
  virtual base::FilePath GetSplashScreenPath();

  XWalkBrowserContext* browser_context_;
  ScopedVector<Runtime> runtimes_;
  scoped_refptr<ApplicationData> const data_;
  // The application's render process host.
  content::RenderProcessHost* render_process_host_;
  content::WebContents* web_contents_;
  bool security_mode_enabled_;

  NativeAppWindow::CreateParams window_show_params_;

  virtual GURL GetStartURL(Manifest::Type type) const;

 private:
  // We enforce ApplicationService ownership.
  friend class ApplicationService;
  static std::unique_ptr<Application> Create(scoped_refptr<ApplicationData> data,
      XWalkBrowserContext* context);

  // content::RenderProcessHostObserver implementation.
  void RenderProcessExited(content::RenderProcessHost* host,
                           base::TerminationStatus status,
                           int exit_code) override;
  void RenderProcessHostDestroyed(
      content::RenderProcessHost* host) override;

  // Try to extract the URL from different possible keys for entry points in the
  // manifest, returns it and the entry point used.
  template <Manifest::Type> GURL GetStartURL() const;

  template <Manifest::Type> void SetWindowShowState(
      NativeAppWindow::CreateParams* params);

  GURL GetAbsoluteURLFromKey(const std::string& key) const;

  void NotifyTermination();

  Observer* observer_;

  std::map<std::string, std::string> name_perm_map_;
  // Application's session permissions.
  StoredPermissionMap permission_map_;
  // Security policy.
  std::unique_ptr<ApplicationSecurityPolicy> security_policy_;
  // WeakPtrFactory should be always declared the last.
  base::WeakPtrFactory<Application> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_H_
