// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/hi_res_timer_manager.h"
#include "base/message_loop.h"
#include "base/power_monitor/power_monitor.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"
#include "content/common/child_process.h"
#include "content/public/common/main_function_params.h"
#include "xwalk/binding/binding/binding_thread.h"
#include "xwalk/binding/common/binding_switches.h"

#if defined(OS_WIN)
#include "base/win/scoped_com_initializer.h"
#endif

namespace xwalk {

// main() routine for running as the binding process.
int BindingMain(const content::MainFunctionParams& parameters) {
  base::MessageLoop main_message_loop(base::MessageLoop::TYPE_UI);
  base::PlatformThread::SetName("BindingMain");

  base::PowerMonitor power_monitor;
  HighResolutionTimerManager high_resolution_timer_manager;

#if defined(OS_WIN)
  base::win::ScopedCOMInitializer com_initializer;
#endif

  const CommandLine& parsed_command_line = parameters.command_line;
  if (parsed_command_line.HasSwitch(switches::kBindingStartupDialog)) {
    content::ChildProcess::WaitForDebugger("Binding");
  }

  {
    content::ChildProcess binding_process;
    binding_process.set_main_thread(new BindingThread());
    base::MessageLoop::current()->Run();
  }

  return 0;
}

}  // namespace xwalk
