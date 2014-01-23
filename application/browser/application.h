// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/observer_list.h"
#include "xwalk/application/browser/event_observer.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/runtime/browser/runtime.h"

namespace xwalk {

class RuntimeContext;

namespace application {

class ApplicationHost;
class Manifest;

// The Application class is representing an active (running) application.
// Application instances are owned by ApplicationService.
// ApplicationService will delete an Application instance when it is
// terminated.
// There's one-to-one correspondence between Application and Render Process
// Host, obtained from its "runtimes" (pages).
class Application : public Runtime::Observer {
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
        entry_points(Default) {}

    LaunchEntryPoints entry_points;
  };

  // Closes all the application's runtimes (application pages).
  // NOTE: ApplicationService deletes an Application instance
  // immediately after its termination.
  void Terminate();

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
  bool IsOnSuspendHandlerRegistered() const;

  RuntimeContext* runtime_context_;
  const scoped_refptr<ApplicationData> application_data_;
  Runtime* main_runtime_;
  std::set<Runtime*> runtimes_;
  scoped_ptr<EventObserver> finish_observer_;
  Observer* observer_;
  // The entry point used as part of Launch().
  LaunchEntryPoint entry_point_used_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_H_
