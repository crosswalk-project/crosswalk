// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts_tizen.h"

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_switches.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "ui/gl/gl_switches.h"
#include "ui/gfx/switches.h"

#include "content/browser/device_sensors/device_inertial_sensor_service.h"
#include "xwalk/tizen/mobile/sensor/tizen_data_fetcher_shared_memory.h"

namespace xwalk {

using application::Application;

XWalkBrowserMainPartsTizen::XWalkBrowserMainPartsTizen(
    const content::MainFunctionParams& parameters)
    : XWalkBrowserMainParts(parameters) {
}

void XWalkBrowserMainPartsTizen::PreMainMessageLoopStart() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  command_line->AppendSwitch(switches::kIgnoreGpuBlacklist);

  // Enable Accelerated 2D Canvas.
  command_line->AppendSwitch(switches::kGpuNoContextLost);

  const char* gl_name;
  if (base::PathExists(base::FilePath("/usr/lib/libGL.so")))
    gl_name = gfx::kGLImplementationDesktopName;
  else
    gl_name = gfx::kGLImplementationEGLName;
  command_line->AppendSwitchASCII(switches::kUseGL, gl_name);

  XWalkBrowserMainParts::PreMainMessageLoopStart();
}

void XWalkBrowserMainPartsTizen::PreMainMessageLoopRun() {
  if (content::DeviceInertialSensorService* sensor_service =
          content::DeviceInertialSensorService::GetInstance()) {
    // As the data fetcher of sensors is implemented outside of Chromium, we
    // need to make it available to Chromium by "abusing" the test framework.
    // TODO(zliang7): Find a decent way to inject our sensor fetcher for Tizen.
    sensor_service->SetDataFetcherForTests(new TizenDataFetcherSharedMemory());
  }

  XWalkBrowserMainParts::PreMainMessageLoopRun();
}

}  // namespace xwalk
