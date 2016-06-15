// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_

#include <memory>
#include <string>

#include "base/memory/scoped_vector.h"
#include "base/values.h"

#include "xwalk/runtime/browser/storage_component.h"

namespace content {
class ContentBrowserClient;
class RenderProcessHost;
}

class XWalkTestSuiteInitializer;

namespace xwalk {

class ApplicationComponent;
class RemoteDebuggingServer;
class SysAppsComponent;
class XWalkBrowserContext;
class XWalkComponent;
class XWalkContentBrowserClient;
class XWalkAppExtensionBridge;

namespace application {
class Application;
class ApplicationSystem;
}

namespace extensions {
class XWalkExtensionService;
};

// Main object for the Browser Process execution in Crosswalk. It is created and
// owned by XWalkMainDelegate. It's role is to own, setup and teardown all the
// subsystems of Crosswalk.
class XWalkRunner {
 public:
  // Read the comments below before using this. Relying too much on this
  // accessor makes the code harder to change and harder to reason about.
  static XWalkRunner* GetInstance();

  virtual ~XWalkRunner();

  // All sub objects should have their dependencies passed during their
  // initialization, so that these accessors below are not frequently accessed.
  // Instead of calling these, consider explicitly passing the dependencies
  // to the objects that need them.
  //
  // For example, if "Application System" needs to use "Runtime Context", we
  // should pass the "Runtime Context" to "Application System" instead of
  // making "Application System" ask XWalkRunner for its dependency.
  //
  // Scenarios when it is fine to use the accessors:
  //
  // - Prototyping solutions, in which we want to see the solution working, and
  //   all dependencies are still not clear. It avoids writing down a lot of
  //   code just to test something out.
  //
  // - In situations where you don't control the creation of a certain
  //   object. Certain APIs doesn't allow us to pass the dependencies, so we
  //   need to reach them some way.
  XWalkBrowserContext* browser_context() { return browser_context_.get(); }
  application::ApplicationSystem* app_system();
  extensions::XWalkExtensionService* extension_service() {
    return extension_service_.get();
  }

  // Stages of main parts. See content/browser_main_parts.h for description.
  virtual void PreMainMessageLoopRun();
  virtual void PostMainMessageLoopRun();

  void EnableRemoteDebugging(int port);
  void DisableRemoteDebugging();

 protected:
  XWalkRunner();

  // These two hooks should be used to add new port specific
  // components. Subclasses *must* call the base class implementation.
  virtual void CreateComponents();
  virtual void DestroyComponents();

  // Should be used by CreateComponents() implementations.
  void AddComponent(std::unique_ptr<XWalkComponent> component);

  // These specific factory functions are used to allow ports to customize
  // components.
  virtual std::unique_ptr<ApplicationComponent> CreateAppComponent();
  virtual std::unique_ptr<SysAppsComponent> CreateSysAppsComponent();
  virtual std::unique_ptr<StorageComponent> CreateStorageComponent();

 protected:
  // These variables are used to export some values from the browser process
  // side to the extension side, such as application IDs and whatnot.
  virtual void InitializeRuntimeVariablesForExtensions(
      const content::RenderProcessHost* host,
      base::ValueMap* runtime_variables);
  virtual void InitializeEnvironmentVariablesForGoogleAPIs(
      content::RenderProcessHost* host);

 private:
  friend class XWalkMainDelegate;
  friend class ::XWalkTestSuiteInitializer;

  // To track OnRenderProcessHostGone.
  friend class application::Application;

  // This class acts as an "arm" of XWalkRunner to fulfill Content API needs,
  // it may call us back in some situations where the a more wider view of the
  // objects is necessary, e.g. during render process lifecycle callbacks.
  friend class XWalkContentBrowserClient;

  // We track the render process lifecycle to register Crosswalk
  // extensions. Some subsystems are mostly implemented using extensions.
  void OnRenderProcessWillLaunch(content::RenderProcessHost* host);
  void OnRenderProcessHostGone(content::RenderProcessHost* host);

  // Create the XWalkRunner object. We use a factory function so that we can
  // switch the concrete class on compile time based on the platform, separating
  // the per-platform behavior and data in the subclasses.
  static std::unique_ptr<XWalkRunner> Create();

  // Note: this is not public as we want to discourage the rest of Crosswalk to
  // rely directly on this object.
  content::ContentBrowserClient* GetContentBrowserClient();

  std::unique_ptr<XWalkContentBrowserClient> content_browser_client_;
  std::unique_ptr<XWalkBrowserContext> browser_context_;
  std::unique_ptr<extensions::XWalkExtensionService> extension_service_;
  std::unique_ptr<XWalkAppExtensionBridge> app_extension_bridge_;

  // XWalkRunner uses the XWalkComponent interface to be able to handle
  // different subsystems and call them in specific situations, e.g. when
  // extensions need to be created.
  ScopedVector<XWalkComponent> components_;

  ApplicationComponent* app_component_;

  // Remote debugger server.
  std::unique_ptr<RemoteDebuggingServer> remote_debugging_server_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRunner);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_
