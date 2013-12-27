// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/runtime/browser/runtime_context.h"
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
  content_browser_client_.reset(new XWalkContentBrowserClient);
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
}

void XWalkRunner::PostMainMessageLoopRun() {
  app_system_.reset();
  runtime_context_.reset();
}

// static
scoped_ptr<XWalkRunner> XWalkRunner::Create() {
  return scoped_ptr<XWalkRunner>(new XWalkRunner);
}

content::ContentBrowserClient* XWalkRunner::GetContentBrowserClient() {
  return content_browser_client_.get();
}

}  // namespace xwalk
