// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/extension_process/xwalk_extension_process_main.h"

#if defined(OS_LINUX)
#include <signal.h>
#include <sys/prctl.h>
#endif

#include "base/logging.h"
#include "base/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/platform_thread.h"
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

  base::RunLoop run_loop;
  run_loop.Run();

  return 0;
}
