// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/extension_process/xwalk_extension_process_main.h"

#if defined(OS_LINUX)
#include <signal.h>
#include <sys/prctl.h>
#endif

#if defined(USE_SMACK)
#include <sys/capability.h>
#include <sys/smack.h>
#endif

#include <string>

#include "base/debug/stack_trace.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/platform_thread.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/extensions/extension_process/xwalk_extension_process.h"

int XWalkExtensionProcessMain(const content::MainFunctionParams& parameters) {
  base::PlatformThread::SetName("XWalkExtensionProcess_Main");

  VLOG(1) << "Extension process running!";

  // FIXME(jeez): This fixes the zombie-process-on-^C issue that we are facing
  // on Linux. However, we must find a cleaner way of doing this, perhaps using
  // something from Chromium and ensuring this process dies when its parent die.
#if defined(OS_LINUX)
  prctl(PR_SET_PDEATHSIG, SIGTERM);
#endif

#if defined(USE_SMACK)
  std::string label = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
     switches::kXWalkWebAppID);
  if (label == "0")
    label = "crosswalk::ep";
  CHECK_EQ(smack_set_label_for_self(label.c_str()), 0);

  cap_t ep_caps = cap_get_proc();
  cap_value_t mac_admin = CAP_MAC_ADMIN;
  CHECK(ep_caps);
  CHECK_EQ(cap_set_flag(ep_caps, CAP_EFFECTIVE, 1, &mac_admin, CAP_CLEAR), 0);
  CHECK_EQ(cap_set_flag(ep_caps, CAP_PERMITTED, 1, &mac_admin, CAP_CLEAR), 0);
  CHECK_EQ(cap_set_proc(ep_caps), 0);
#endif

  // On Linux-based platforms, we want the Glib message pump running so we need
  // a TYPE_UI MessageLoop. For other platforms we will stick with TYPE_DEFAULT
  // for now.
  base::MessageLoop::Type message_loop_type;
#if defined(OS_POSIX)
  message_loop_type = base::MessageLoop::TYPE_UI;
#else
  message_loop_type = base::MessageLoop::TYPE_DEFAULT;
#endif

  base::MessageLoop main_message_loop(message_loop_type);
  xwalk::extensions::XWalkExtensionProcess extension_process;

#ifndef NDEBUG
  // In debug mode, enable to stack dumping in a similar fashion than Renderer
  // Process. This is helpful to understand crashes in Extension Process.
  base::debug::EnableInProcessStackDumping();
#endif

  base::RunLoop run_loop;
  run_loop.Run();

  return 0;
}
