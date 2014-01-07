// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"

namespace content {
class ContentBrowserClient;
class RenderProcessHost;
}

class XWalkTestSuiteInitializer;

namespace xwalk {

class RuntimeContext;
class ApplicationComponent;
class XWalkComponent;
class XWalkContentBrowserClient;

namespace application {
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
  RuntimeContext* runtime_context() { return runtime_context_.get(); }
  application::ApplicationSystem* app_system();
  extensions::XWalkExtensionService* extension_service() {
    return extension_service_.get();
  }

  // Return true if Crosswalk is running in service mode, i.e. taking
  // requests from native IPC mechanism to launch applications.
  bool is_running_as_service() const { return is_running_as_service_; }

  // Stages of main parts. See content/browser_main_parts.h for description.
  void PreMainMessageLoopRun();
  void PostMainMessageLoopRun();

 protected:
  XWalkRunner();

  // These two hooks should be used to add new port specific
  // components. Subclasses *must* call the base class implementation.
  virtual void CreateComponents();
  virtual void DestroyComponents();

  // Should be used by CreateComponents() implementations.
  void AddComponent(scoped_ptr<XWalkComponent> component);

  // These specific factory functions are used to allow ports to customize
  // components.
  virtual scoped_ptr<ApplicationComponent> CreateAppComponent();

 private:
  friend class XWalkMainDelegate;
  friend class ::XWalkTestSuiteInitializer;

  // To track OnRenderProcessHostGone.
  friend class Runtime;

  // This class acts as an "arm" of XWalkRunner to fulfill Content API needs,
  // it may call us back in some situations where the a more wider view of the
  // objects is necessary, e.g. during render process lifecycle callbacks.
  friend class XWalkContentBrowserClient;

  // We track the render process lifecycle to register Crosswalk
  // extensions. Some subsystems are mostly implemented using extensions.
  void OnRenderProcessHostCreated(content::RenderProcessHost* host);
  void OnRenderProcessHostGone(content::RenderProcessHost* host);

  // Create the XWalkRunner object. We use a factory function so that we can
  // switch the concrete class on compile time based on the platform, separating
  // the per-platform behavior and data in the subclasses.
  static scoped_ptr<XWalkRunner> Create();

  // Note: this is not public as we want to discourage the rest of Crosswalk to
  // rely directly on this object.
  content::ContentBrowserClient* GetContentBrowserClient();

  scoped_ptr<XWalkContentBrowserClient> content_browser_client_;
  scoped_ptr<RuntimeContext> runtime_context_;
  scoped_ptr<extensions::XWalkExtensionService> extension_service_;

  // XWalkRunner uses the XWalkComponent interface to be able to handle
  // different subsystems and call them in specific situations, e.g. when
  // extensions need to be created.
  ScopedVector<XWalkComponent> components_;

  ApplicationComponent* app_component_;

  bool is_running_as_service_;

  DISALLOW_COPY_AND_ASSIGN(XWalkRunner);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_RUNNER_H_
