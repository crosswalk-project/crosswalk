// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts_tizen.h"

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "content/public/common/content_switches.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/runtime/extension/runtime_extension.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"
#include "ui/gl/gl_switches.h"

#include "content/browser/device_orientation/device_inertial_sensor_service.h"
#include "xwalk/application/browser/installer/tizen/package_installer.h"
#include "xwalk/runtime/browser/tizen/tizen_data_fetcher_shared_memory.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_extension.h"

namespace xwalk {

XWalkBrowserMainPartsTizen::XWalkBrowserMainPartsTizen(
    const content::MainFunctionParams& parameters)
    : XWalkBrowserMainParts(parameters) {
}

void XWalkBrowserMainPartsTizen::PreMainMessageLoopStart() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  command_line->AppendSwitch(switches::kIgnoreGpuBlacklist);

  const char* gl_name;
  if (base::PathExists(base::FilePath("/usr/lib/xwalk/libosmesa.so")))
    gl_name = gfx::kGLImplementationOSMesaName;
  else if (base::PathExists(base::FilePath("/usr/lib/libGL.so")))
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

void
XWalkBrowserMainPartsTizen::RegisterInternalExtensionsInExtensionThreadServer(
    extensions::XWalkExtensionServer* server) {
  CHECK(server);
  server->RegisterExtension(scoped_ptr<extensions::XWalkExtension>(
      new sysapps::DeviceCapabilitiesExtension(runtime_registry_.get())));
  server->RegisterExtension(scoped_ptr<extensions::XWalkExtension>(
      new sysapps::RawSocketExtension()));
}

}  // namespace xwalk
