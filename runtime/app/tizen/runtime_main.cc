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

#if defined(OS_TIZEN_DESKTOP)
#include "xwalk/runtime/app/tizen/app_main_desktop_mock.h"
#else
#include <app.h>  // NOLINT(build/include_order)
#endif

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

///////////////////////////////// Implementation for app_efl_main()

struct UserData {
  scoped_ptr<WebRuntimeContext> context;
};

bool Create(void* data) {
  UserData* user_data = static_cast<UserData*>(data);

  if (!user_data)
    return false;

  user_data->context.reset(new WebRuntimeContext);
  return true;
}

void Terminate(void* data) {
  UserData* user_data = static_cast<UserData*>(data);

  if (!user_data)
    return;

  user_data->context.reset();
}

void Pause(void* data) {
}

void Resume(void* data) {
}

void Service(service_h service, void* data) {
}

void LowMemory(void* data) {
}

void LowBattery(void* data) {
}

void DevOrientationChanged(app_device_orientation_e orientation,
                              void* data) {
}

void LangChanged(void* data) {
}

void RegionFmtChanged(void* data) {
}

}  // namespace

int RuntimeMain(int argc, const char** argv) {
  UserData user_data;

  app_event_callback_s callbasks;
  callbasks.create = Create;
  callbasks.terminate = Terminate;
  callbasks.pause = Pause;
  callbasks.resume = Resume;
  callbasks.service = Service;
  callbasks.low_memory = LowMemory;
  callbasks.low_battery = LowBattery;
  callbasks.device_orientation = DevOrientationChanged;
  callbasks.language_changed = LangChanged;
  callbasks.region_format_changed = RegionFmtChanged;

  app_efl_main(&argc, const_cast<char***>(&argv), &callbasks, &user_data);
  return 0;
}

}  // namespace xwalk
