// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner.h"

#include <string>
#include <vector>
#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/runtime/browser/application_component.h"
#include "xwalk/runtime/browser/devtools/remote_debugging_server.h"
#include "xwalk/runtime/browser/storage_component.h"
#include "xwalk/runtime/browser/sysapps_component.h"
#include "xwalk/runtime/browser/xwalk_app_extension_bridge.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/xwalk_component.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_WIN)
#include "xwalk/runtime/browser/xwalk_runner_win.h"
#endif

namespace xwalk {

namespace {

XWalkRunner* g_xwalk_runner = NULL;

}  // namespace

XWalkRunner::XWalkRunner()
    : app_component_(nullptr) {
  VLOG(1) << "Creating XWalkRunner object.";
  DCHECK(!g_xwalk_runner);
  g_xwalk_runner = this;

  XWalkRuntimeFeatures::GetInstance()->Initialize(
      base::CommandLine::ForCurrentProcess());

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
  browser_context_.reset(new XWalkBrowserContext);
  app_extension_bridge_.reset(new XWalkAppExtensionBridge());

  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkDisableExtensions))
    extension_service_.reset(new extensions::XWalkExtensionService(
        app_extension_bridge_.get()));

  CreateComponents();
  app_extension_bridge_->SetApplicationSystem(app_component_->app_system());
  browser_context_->set_application_service(
      app_system()->application_service());
}

void XWalkRunner::PostMainMessageLoopRun() {
  DestroyComponents();
  extension_service_.reset();
  browser_context_.reset();
  DisableRemoteDebugging();
}

void XWalkRunner::CreateComponents() {
  std::unique_ptr<ApplicationComponent> app_component(CreateAppComponent());
  // Keep a reference as some code still needs to call
  // XWalkRunner::app_system().
  app_component_ = app_component.get();
  AddComponent(std::move(app_component));

  if (XWalkRuntimeFeatures::isSysAppsEnabled())
    AddComponent(CreateSysAppsComponent());
  if (XWalkRuntimeFeatures::isStorageAPIEnabled())
    AddComponent(CreateStorageComponent());
}

void XWalkRunner::DestroyComponents() {
  // The ScopedVector takes care of deleting all the components. Ensure that
  // components are deleted before their dependencies by reversing the order.
  std::reverse(components_.begin(), components_.end());
  components_.clear();

  app_component_ = NULL;
}

void XWalkRunner::AddComponent(std::unique_ptr<XWalkComponent> component) {
  components_.push_back(component.release());
}

std::unique_ptr<ApplicationComponent> XWalkRunner::CreateAppComponent() {
  return base::WrapUnique(new ApplicationComponent(browser_context_.get()));
}

std::unique_ptr<SysAppsComponent> XWalkRunner::CreateSysAppsComponent() {
  return base::WrapUnique(new SysAppsComponent());
}

std::unique_ptr<StorageComponent> XWalkRunner::CreateStorageComponent() {
  return base::WrapUnique(new StorageComponent());
}

void XWalkRunner::InitializeRuntimeVariablesForExtensions(
    const content::RenderProcessHost* host,
    base::DictionaryValue::Storage* variables) {
  application::Application* app = app_system()->application_service()->
      GetApplicationByRenderHostID(host->GetID());

  if (app)
    (*variables)["app_id"] =
        base::WrapUnique(new base::StringValue(app->id()));
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

  InitializeEnvironmentVariablesForGoogleAPIs(host);

  std::unique_ptr<base::DictionaryValue::Storage>
      runtime_variables(new base::DictionaryValue::Storage);
  InitializeRuntimeVariablesForExtensions(host, runtime_variables.get());
  extension_service_->OnRenderProcessWillLaunch(
      host, &ui_thread_extensions, &extension_thread_extensions,
      std::move(runtime_variables));
}

void XWalkRunner::InitializeEnvironmentVariablesForGoogleAPIs(
    content::RenderProcessHost* host) {
}

void XWalkRunner::OnRenderProcessHostGone(content::RenderProcessHost* host) {
  if (!extension_service_)
    return;
  extension_service_->OnRenderProcessDied(host);
}

void XWalkRunner::EnableRemoteDebugging(int port) {
  const char* local_ip = "0.0.0.0";
  if (port > 0 && port < 65535) {
    if (remote_debugging_server_.get() &&
        remote_debugging_server_.get()->port() == port)
      remote_debugging_server_.reset();
    remote_debugging_server_.reset(
        new RemoteDebuggingServer(browser_context(),
            local_ip, port, std::string()));
  }
}

void XWalkRunner::DisableRemoteDebugging() {
  remote_debugging_server_.reset();
}

// static
std::unique_ptr<XWalkRunner> XWalkRunner::Create() {
#if defined (OS_WIN)
  return std::unique_ptr<XWalkRunner>(new XWalkRunnerWin);
#else
  return std::unique_ptr<XWalkRunner>(new XWalkRunner);
#endif
}

content::ContentBrowserClient* XWalkRunner::GetContentBrowserClient() {
  return content_browser_client_.get();
}

}  // namespace xwalk
