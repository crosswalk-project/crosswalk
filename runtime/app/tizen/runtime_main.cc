// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/tizen/runtime_main.h"

#include <Ecore.h>
#include <Elementary.h>
#include "base/bind.h"
#include "base/memory/scoped_ptr.h"
#include "base/run_loop.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/main_function_params.h"
#include "xwalk/runtime/app/tizen/message_pump_efl.h"
#include "xwalk/runtime/app/xwalk_main_delegate.h"

namespace xwalk {

namespace {

base::MessagePump* MessagePumpFactory() {
  return new base::MessagePumpEFL;
}

class WebRuntimeContext {
 public:
  WebRuntimeContext();
  ~WebRuntimeContext();

 private:
  scoped_ptr<content::ContentMainRunner> runner_;
  scoped_ptr<content::BrowserMainRunner> main_runner_;

  DISALLOW_COPY_AND_ASSIGN(WebRuntimeContext);
};

void DummyRun() {
}

WebRuntimeContext::WebRuntimeContext() {
  // MessagePumpGlib uses glib main context,
  // so we need to integrate glib with ecore main loop.
  ecore_main_loop_glib_integrate();
  // Inject MessagePumpFacotry for embedder.
  base::MessageLoop::InitMessagePumpForUIFactory(MessagePumpFactory);

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
}

WebRuntimeContext::~WebRuntimeContext() {
  main_runner_->Shutdown();
  runner_->Shutdown();
}

}  // namespace

int RuntimeMain(int argc, const char** argv) {
  elm_init(argc, const_cast<char**>(argv));
  scoped_ptr<WebRuntimeContext> context(new WebRuntimeContext);
  elm_run();
  return 0;
}

}  // namespace xwalk
