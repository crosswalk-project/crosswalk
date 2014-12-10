// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_
#define XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_

#include <appcore-common.h>
#include <glib.h>

#include <memory>
#include <string>

#include "base/message_loop/message_loop.h"

#include "xwalk/application/common/id_util.h"
#include "xwalk/application/tools/linux/xwalk_launcher.h"
#include "xwalk/application/tools/tizen/xwalk_tizen_user.h"

// Private enum from appcore-internal
extern "C" enum app_event {
  AE_UNKNOWN,
  AE_CREATE,
  AE_TERMINATE,
  AE_PAUSE,
  AE_RESUME,
  AE_RESET,
  AE_LOWMEM_POST,
  AE_MEM_FLUSH,
  AE_MAX
};

class XWalkLauncherTizen : public XWalkLauncher {
 public:
  XWalkLauncherTizen(bool query_running, base::MessageLoop* main_loop);
  int Launch(const std::string& appid_or_url, bool fullscreen,
             bool remote_debugging, int argc, char* argv[]);
  bool Suspend();
  bool Resume();

 private:
  static void application_event_cb(app_event event, void* data, bundle* b);
  int XwalkAppcoreInit(const std::string& name, int argc, char* argv[]);
  int XwalkChangeCmdline(const std::string& app_id, int argc, char* argv[]);

  base::MessageLoop* main_loop_;
};

#endif  // XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_TIZEN_H_
