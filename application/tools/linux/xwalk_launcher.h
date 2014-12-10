// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_H_
#define XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_H_

#include <memory>
#include <string>

#include "base/threading/thread.h"
#include "dbus/bus.h"
#include "dbus/message.h"

#include "xwalk/application/tools/linux/dbus_object_manager.h"
#include "xwalk/application/tools/linux/xwalk_extension_process_launcher.h"

class XWalkLauncher : public DBusObjectManager::Observer {
 public:
  XWalkLauncher(bool query_running, base::MessageLoop* main_loop);
  virtual ~XWalkLauncher();
  virtual int Launch(const std::string& appid_or_url, bool fullscreen,
                     bool remote_debugging, int argc = 0,
                     char* argv[] = nullptr);

 protected:
  int LaunchApplication();

  std::unique_ptr<XWalkExtensionProcessLauncher> ep_launcher_;

  unsigned int launcher_pid_;
  std::string appid_or_url_;
  bool fullscreen_;
  bool remote_debugging_;
  bool query_running_;

  base::MessageLoop* main_loop_;

  std::unique_ptr<DBusObjectManager> dbus_object_manager_;

 private:
  bool InitExtensionProcessChannel();
  virtual void OnEPChannelCreated() override;

  std::unique_ptr<base::Thread> dbus_thread_;
};

#endif  // XWALK_APPLICATION_TOOLS_LINUX_XWALK_LAUNCHER_H_
