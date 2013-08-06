// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/tizen/runtime_main.h"

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"
#include "base/run_loop.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/main_function_params.h"
#include "xwalk/runtime/app/xwalk_main_delegate.h"

namespace xwalk {

namespace {

class WebRuntimeContext {
 public:
  WebRuntimeContext();
  ~WebRuntimeContext();
  void Run();

 private:
  scoped_ptr<content::ContentMainRunner> runner_;
  scoped_ptr<content::BrowserMainRunner> main_runner_;
  scoped_ptr<base::RunLoop> run_loop_;

  DISALLOW_COPY_AND_ASSIGN(WebRuntimeContext);
};

void DummyRun() {
}

WebRuntimeContext::WebRuntimeContext() {
  runner_.reset(content::ContentMainRunner::Create());
  runner_->Initialize(0, 0, new XWalkMainDelegate);

  main_runner_.reset(content::BrowserMainRunner::Create());
  content::MainFunctionParams params(*CommandLine::ForCurrentProcess());
  // We make Chromium know that embedder will run the ui loop.
  // However, we will run the ui loop in RuntimeMain() instead of DummyRun().
  // See XWalkBrowserMainParts::PreMainMessageLoopRun().
  params.ui_task = new base::Closure(base::Bind(DummyRun));

  int result_code = main_runner_->Initialize(params);
  CHECK_LT(result_code, 0);
  main_runner_->Run();

  // Once the MessageLoop has been created, attach a top-level RunLoop.
  run_loop_.reset(new base::RunLoop);
}

WebRuntimeContext::~WebRuntimeContext() {
  main_runner_->Shutdown();
  runner_->Shutdown();
}

void WebRuntimeContext::Run() {
  run_loop_->Run();
}

}  // namespace

int RuntimeMain(int argc, const char** argv) {
  {
    scoped_ptr<WebRuntimeContext> context(new WebRuntimeContext);
    context->Run();
  }
  return 0;
}

}  // namespace xwalk
