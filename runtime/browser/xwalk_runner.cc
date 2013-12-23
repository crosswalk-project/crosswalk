// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner.h"

#include <vector>
#include "base/command_line.h"
#include "base/logging.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {

namespace {

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

void XWalkRunner::PreMainMessageLoopRun() {
  runtime_context_.reset(new RuntimeContext);
  app_system_ =
      application::ApplicationSystem::Create(runtime_context_.get());

  // FIXME(cmarcelo): Remove this check once we remove the --uninstall
  // command line.
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kUninstall))
    extension_service_.reset(new extensions::XWalkExtensionService);
}

void XWalkRunner::PostMainMessageLoopRun() {
  extension_service_.reset();
  app_system_.reset();
  runtime_context_.reset();
}

void XWalkRunner::OnRenderProcessHostCreated(content::RenderProcessHost* host) {
  if (!extension_service_)
    return;

  // TODO(cmarcelo): Move Create*Extensions*() functions to XWalkRunner.
  XWalkBrowserMainParts* main_parts = content_browser_client_->main_parts();
  std::vector<extensions::XWalkExtension*> ui_thread_extensions;
  main_parts->CreateInternalExtensionsForUIThread(
      host, &ui_thread_extensions);

  std::vector<extensions::XWalkExtension*> extension_thread_extensions;
  main_parts->CreateInternalExtensionsForExtensionThread(
      host, &extension_thread_extensions);

  extension_service_->OnRenderProcessHostCreated(
      host, &ui_thread_extensions, &extension_thread_extensions);
}

void XWalkRunner::OnRenderProcessHostGone(content::RenderProcessHost* host) {
  if (!extension_service_)
    return;
  extension_service_->OnRenderProcessDied(host);
}

// static
scoped_ptr<XWalkRunner> XWalkRunner::Create() {
  return scoped_ptr<XWalkRunner>(new XWalkRunner);
}

content::ContentBrowserClient* XWalkRunner::GetContentBrowserClient() {
  return content_browser_client_.get();
}

}  // namespace xwalk
