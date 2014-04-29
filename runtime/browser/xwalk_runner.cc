// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner.h"

#include <vector>
#include "base/command_line.h"
#include "base/logging.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/runtime/browser/application_component.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/sysapps_component.h"
#include "xwalk/runtime/browser/xwalk_app_extension_bridge.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/xwalk_component.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/xwalk_runner_android.h"
#elif defined(OS_TIZEN)
#include "xwalk/runtime/browser/xwalk_runner_tizen.h"
#endif

namespace xwalk {

namespace {

const char kDefaultLocale[] = "en-US";
XWalkRunner* g_xwalk_runner = NULL;

}  // namespace

XWalkRunner::XWalkRunner()
    : is_running_as_service_(false) {
  VLOG(1) << "Creating XWalkRunner object.";
  DCHECK(!g_xwalk_runner);
  g_xwalk_runner = this;

  XWalkRuntimeFeatures::GetInstance()->Initialize(
      CommandLine::ForCurrentProcess());
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  is_running_as_service_ = cmd_line->HasSwitch(switches::kXWalkRunAsService);

  // Initializing after the g_xwalk_runner is set to ensure
  // XWalkRunner::GetInstance() can be used in all sub objects if needed.
  content_browser_client_.reset(new XWalkContentBrowserClient(this));
}

XWalkRunner::~XWalkRunner() {
  DCHECK(g_xwalk_runner);
  g_xwalk_runner = NULL;
  VLOG(1) << "Destroying XWalkRunner object.";
}

// static
XWalkRunner* XWalkRunner::GetInstance() {
  return g_xwalk_runner;
}

application::ApplicationSystem* XWalkRunner::app_system() {
  return app_component_ ? app_component_->app_system() : NULL;
}

void XWalkRunner::PreMainMessageLoopRun() {
  runtime_context_.reset(new RuntimeContext);
  app_extension_bridge_.reset(new XWalkAppExtensionBridge());
  // FIXME(cmarcelo): Remove this check once we remove the --uninstall
  // command line.
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kUninstall))
    extension_service_.reset(new extensions::XWalkExtensionService(
        app_extension_bridge_.get()));
  CreateComponents();
  app_extension_bridge_->SetApplicationSystem(app_component_->app_system());
}

void XWalkRunner::PostMainMessageLoopRun() {
  DestroyComponents();
  extension_service_.reset();
  runtime_context_.reset();
}

std::string XWalkRunner::GetLocale() const {
  return kDefaultLocale;
}

void XWalkRunner::CreateComponents() {
  scoped_ptr<ApplicationComponent> app_component(CreateAppComponent());
  // Keep a reference as some code still needs to call
  // XWalkRunner::app_system().
  app_component_ = app_component.get();
  AddComponent(app_component.PassAs<XWalkComponent>());

  if (XWalkRuntimeFeatures::isSysAppsEnabled())
    AddComponent(CreateSysAppsComponent().PassAs<XWalkComponent>());
}

void XWalkRunner::DestroyComponents() {
  // The ScopedVector takes care of deleting all the components. Ensure that
  // components are deleted before their dependencies by reversing the order.
  std::reverse(components_.begin(), components_.end());
  components_.clear();

  app_component_ = NULL;
}

void XWalkRunner::AddComponent(scoped_ptr<XWalkComponent> component) {
  components_.push_back(component.release());
}

scoped_ptr<ApplicationComponent> XWalkRunner::CreateAppComponent() {
  return make_scoped_ptr(new ApplicationComponent(runtime_context_.get()));
}

scoped_ptr<SysAppsComponent> XWalkRunner::CreateSysAppsComponent() {
  return make_scoped_ptr(new SysAppsComponent());
}

void XWalkRunner::InitializeRuntimeVariablesForExtensions(
    const content::RenderProcessHost* host,
    base::ValueMap& variables) {
  application::Application* app = app_system()->application_service()->
      GetApplicationByRenderHostID(host->GetID());

  if (app)
    variables["app_id"] = base::Value::CreateStringValue(app->id());
}

void XWalkRunner::OnRenderProcessWillLaunch(content::RenderProcessHost* host) {
  if (!extension_service_)
    return;

  std::vector<extensions::XWalkExtension*> ui_thread_extensions;
  std::vector<extensions::XWalkExtension*> extension_thread_extensions;

  ScopedVector<XWalkComponent>::iterator it = components_.begin();
  for (; it != components_.end(); ++it) {
    XWalkComponent* component = *it;
    component->CreateUIThreadExtensions(host, &ui_thread_extensions);
    component->CreateExtensionThreadExtensions(
        host, &extension_thread_extensions);
  }

  // TODO(cmarcelo): Once functionality is moved to components, remove
  // CreateInternalExtensions*() functions from XWalkBrowserMainParts.
  XWalkBrowserMainParts* main_parts = content_browser_client_->main_parts();
  main_parts->CreateInternalExtensionsForUIThread(
      host, &ui_thread_extensions);
  main_parts->CreateInternalExtensionsForExtensionThread(
      host, &extension_thread_extensions);

  base::ValueMap runtime_variables;
  InitializeRuntimeVariablesForExtensions(host, runtime_variables);
  extension_service_->OnRenderProcessWillLaunch(
      host, &ui_thread_extensions, &extension_thread_extensions,
      runtime_variables);
}

void XWalkRunner::OnRenderProcessHostGone(content::RenderProcessHost* host) {
  if (!extension_service_)
    return;
  extension_service_->OnRenderProcessDied(host);
}

// static
scoped_ptr<XWalkRunner> XWalkRunner::Create() {
  XWalkRunner* runner = NULL;
#if defined(OS_ANDROID)
  runner = new XWalkRunnerAndroid;
#elif defined(OS_TIZEN)
  runner = new XWalkRunnerTizen;
#else
  runner = new XWalkRunner;
#endif
  return scoped_ptr<XWalkRunner>(runner);
}

content::ContentBrowserClient* XWalkRunner::GetContentBrowserClient() {
  return content_browser_client_.get();
}

}  // namespace xwalk
