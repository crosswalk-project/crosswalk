// Copyright (c) 2013 Intel Corporation. All rights reserved.
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
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/observer_list.h"
#include "ui/base/ui_base_types.h"
#include "xwalk/application/browser/event_observer.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/security_policy.h"
#include "xwalk/runtime/browser/runtime.h"

#if defined(USE_OZONE) && defined(OS_TIZEN)
#include "base/message_loop/message_pump_observer.h"
#endif

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
class Application
  :
#if defined(USE_OZONE) && defined(OS_TIZEN)
  public base::MessagePumpObserver,
#endif
  public Runtime::Observer {
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

  // Manifest keys that can be used as application entry points.
  enum LaunchEntryPoint {
    AppMainKey = 1 << 0,  // app.main
    LaunchLocalPathKey = 1 << 1,  // app.launch.local_path
    // NOTE: The following key is only used for "dummy" hosted apps,
    // which can be using any arbitrary URL, incl. remote ones.
    // For now this should be disabled for all other cases as this will
    // require special care with permissions etc.
    URLKey = 1 << 2,  // url
    Default = AppMainKey | LaunchLocalPathKey
  };
  typedef unsigned LaunchEntryPoints;

  struct LaunchParams {
    LaunchParams() :
        entry_points(Default),
        launcher_pid(0),
        window_state(ui::SHOW_STATE_DEFAULT) {}

    LaunchEntryPoints entry_points;

    // Used only when running as service. Specifies the PID of the launcher
    // process.
    int32 launcher_pid;

    // Sets the initial state for the application windows.
    ui::WindowShowState window_state;
  };

  // Closes all the application's runtimes (application pages).
  // NOTE: Application is terminated asynchronously.
  // Please use ApplicationService::Observer::WillDestroyApplication()
  // interface to be notified about actual app termination.
  //
  // NOTE: ApplicationService deletes an Application instance
  // immediately after its termination.
  enum TerminationMode {
    Normal,
    Immediate  // Ignore OnSuspend event handler.
  };
  void Terminate(TerminationMode = Normal);

  // Returns Runtime (application page) containing the application's
  // 'main document'. The main document is the main entry point of
  // the application to the system. This method will return 'NULL'
  // if application has different entry point (local path manifest key).
  // See http://anssiko.github.io/runtime/app-lifecycle.html#dfn-main-document
  // The main document never has a visible window on its own.
  Runtime* GetMainDocumentRuntime() const;

  const std::set<Runtime*>& runtimes() const { return runtimes_; }

  // Returns the unique application id which is used to distinguish the
  // application amoung both running applications and installed ones
  // (ApplicationData objects).
  std::string id() const { return application_data_->ID(); }
  int GetRenderProcessHostID() const;

  const ApplicationData* data() const { return application_data_; }
  ApplicationData* data() { return application_data_; }

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
                                 std::string& permission_name) const;
  bool SetPermission(PermissionType type,
                     const std::string& permission_name,
                     StoredPermission perm);
  bool CanRequestURL(const GURL& url) const;

 private:
  bool HasMainDocument() const;
  // Runtime::Observer implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE;

  // We enforce ApplicationService ownership.
  friend class ApplicationService;
  Application(scoped_refptr<ApplicationData> data,
              RuntimeContext* context,
              Observer* observer);
  bool Launch(const LaunchParams& launch_params);

  // Try to extract the URL from different possible keys for entry points in the
  // manifest, returns it and the entry point used.
  GURL GetURLForLaunch(const LaunchParams& params, LaunchEntryPoint* used);

  GURL GetURLFromAppMainKey();
  GURL GetURLFromLocalPathKey();
  GURL GetURLFromURLKey();

  friend class FinishEventObserver;
  void CloseMainDocument();
  void NotifyTermination();
  bool IsOnSuspendHandlerRegistered() const;
  bool IsTerminating() const { return finish_observer_; }

  void InitSecurityPolicy();
  void AddSecurityPolicy(const GURL& url, bool subdomains);

#if defined(USE_OZONE) && defined(OS_TIZEN)
  virtual base::EventStatus WillProcessEvent(
      const base::NativeEvent& event) OVERRIDE;
  virtual void DidProcessEvent(const base::NativeEvent& event) OVERRIDE;
#endif

  RuntimeContext* runtime_context_;
  const scoped_refptr<ApplicationData> application_data_;
  Runtime* main_runtime_;
  std::set<Runtime*> runtimes_;
  scoped_ptr<EventObserver> finish_observer_;
  Observer* observer_;
  // The entry point used as part of Launch().
  LaunchEntryPoint entry_point_used_;
  TerminationMode termination_mode_used_;
  base::WeakPtrFactory<Application> weak_factory_;
  std::map<std::string, std::string> name_perm_map_;
  // Application's session permissions.
  StoredPermissionMap permission_map_;
  // Security policy set.
  ScopedVector<SecurityPolicy> security_policy_;
  bool is_security_mode_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_H_
