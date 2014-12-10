// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/linux/xwalk_launcher_tizen.h"

#include <unistd.h>
#include <pkgmgr-info.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "base/logging.h"
#include "url/gurl.h"

// Private struct from appcore-internal, necessary to get events from
// the system.
struct ui_ops {
  void* data;
  void (*cb_app)(app_event evnt, void* data, bundle* b);
};

namespace {

const char* Event2Str(app_event event) {
  switch (event) {
    case AE_UNKNOWN:
      return "AE_UNKNOWN";
    case AE_CREATE:
      return "AE_CREATE";
    case AE_TERMINATE:
      return "AE_TERMINATE";
    case AE_PAUSE:
      return "AE_PAUSE";
    case AE_RESUME:
      return "AE_RESUME";
    case AE_RESET:
      return "AE_RESET";
    case AE_LOWMEM_POST:
      return "AE_LOWMEM_POST";
    case AE_MEM_FLUSH:
      return "AE_MEM_FLUSH";
    case AE_MAX:
      return "AE_MAX";
  }

  return "INVALID EVENT";
}

ui_ops app_ops;

}  // namespace

XWalkLauncherTizen::XWalkLauncherTizen(bool query_running,
    base::MessageLoop* main_loop)
    : XWalkLauncher(query_running, main_loop) {
}

int XWalkLauncherTizen::Launch(const std::string& appid_or_url, bool fullscreen,
                               bool remote_debugging, int argc, char* argv[]) {
  appid_or_url_ = appid_or_url;
  fullscreen_ = fullscreen;
  remote_debugging_ = remote_debugging;
  // Query app.
  if (query_running_) {
    return dbus_object_manager_->IsApplicationRunning(appid_or_url_);
  }
  std::string name = "xwalk-" + appid_or_url_;

  if (XwalkAppcoreInit(name, argc, argv)) {
    LOG(ERROR) << "Failed to initialize appcore.";
    return 1;
  }
  if (GURL(appid_or_url_).spec().empty()
      && XwalkChangeCmdline(appid_or_url_, argc, argv))
    return 1;
  return 0;
}

bool XWalkLauncherTizen::Suspend() {
  return dbus_object_manager_->Suspend();
}

bool XWalkLauncherTizen::Resume() {
  return dbus_object_manager_->Resume();
}

void XWalkLauncherTizen::application_event_cb(app_event event,
                                              void* data, bundle* b) {
  XWalkLauncherTizen* xwalk_launcher = static_cast<XWalkLauncherTizen*>(data);
  LOG(INFO) << "event '" << Event2Str(event) << "'";

  switch (event) {
    case AE_UNKNOWN:
    case AE_CREATE:
      break;
    case AE_TERMINATE:
      xwalk_launcher->main_loop_->QuitNow();
      break;
    case AE_PAUSE:
      if (!xwalk_launcher->Suspend())
        LOG(ERROR) << "Suspending application failed";
      break;
    case AE_RESUME:
      if (!xwalk_launcher->Resume())
        LOG(ERROR) << "Resuming application failed";
      break;
    case AE_RESET:
      if (!xwalk_launcher->LaunchApplication())
        xwalk_launcher->main_loop_->QuitNow();
      break;
    case AE_LOWMEM_POST:
    case AE_MEM_FLUSH:
    case AE_MAX:
      break;
  }
}

int XWalkLauncherTizen::XwalkAppcoreInit(const std::string& name,
                                         int argc, char* argv[]) {
  app_ops.cb_app = application_event_cb;
  app_ops.data = this;
  return appcore_init(name.c_str(), &app_ops, argc, argv);
}

int XWalkLauncherTizen::XwalkChangeCmdline(const std::string& app_id,
                                           int argc, char* argv[]) {
  // Change /proc/<pid>/cmdline to app exec path. See XWALK-1722 for details.
  pkgmgrinfo_appinfo_h handle;
  char* exec_path = nullptr;
  // todo : add is_admin
  if (pkgmgrinfo_appinfo_get_usr_appinfo(app_id.c_str(),
      getuid(), &handle) != PMINFO_R_OK ||
      pkgmgrinfo_appinfo_get_exec(handle, &exec_path) != PMINFO_R_OK ||
      !exec_path) {
    if (pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle) !=
        PMINFO_R_OK ||
        pkgmgrinfo_appinfo_get_exec(handle, &exec_path) != PMINFO_R_OK ||
        !exec_path) {
      LOG(ERROR) << "Couldn't find exec path for application: " << app_id;
      return -1;
    }
  }

  // zeros g_argv_
  for (int i = 0; i < argc; ++i)
    memset(argv[i], 0, strlen(argv[i]));

  strncpy(argv[0], exec_path, strlen(exec_path) + 1);
  pkgmgrinfo_appinfo_destroy_appinfo(handle);
  return 0;
}
