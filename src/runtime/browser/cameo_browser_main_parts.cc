// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/cameo_browser_main_parts.h"

#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "cameo/src/runtime/browser/runtime.h"
#include "cameo/src/runtime/browser/runtime_context.h"
#include "cameo/src/runtime/browser/runtime_registry.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "net/base/net_util.h"

namespace cameo {

CameoBrowserMainParts::CameoBrowserMainParts(
    const content::MainFunctionParams& parameters)
    : BrowserMainParts(),
      startup_url_(chrome::kAboutBlankURL),
      parameters_(parameters),
      run_default_message_loop_(true) {
}

CameoBrowserMainParts::~CameoBrowserMainParts() {
}

void CameoBrowserMainParts::PreMainMessageLoopStart() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  const CommandLine::StringVector& args = command_line->GetArgs();

  if (args.empty())
    return;

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme())
    startup_url_ = url;
  else
    startup_url_ = net::FilePathToFileURL(base::FilePath(args[0]));
}

void CameoBrowserMainParts::PostMainMessageLoopStart() {
}

void CameoBrowserMainParts::PreEarlyInitialization() {
}

void CameoBrowserMainParts::PreMainMessageLoopRun() {
  runtime_context_.reset(new RuntimeContext);
  runtime_registry_.reset(new RuntimeRegistry);

  // The new created Runtime instance will be managed by RuntimeRegistry.
  Runtime::Create(runtime_context_.get(), startup_url_);

  // If the |ui_task| is specified in main function parameter, it indicates
  // that we will run this UI task instead of running the the default main
  // message loop. See |content::BrowserTestBase::SetUp| for |ui_task| usage
  // case.
  if (parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_default_message_loop_ = false;
  }
}

bool CameoBrowserMainParts::MainMessageLoopRun(int* result_code) {
  return !run_default_message_loop_;
}

void CameoBrowserMainParts::PostMainMessageLoopRun() {
  runtime_context_.reset();
}

}  // cameo
