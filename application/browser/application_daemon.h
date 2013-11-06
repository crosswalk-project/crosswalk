// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_DAEMON_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_DAEMON_H_

#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {
namespace application {

// ApplicationDaemon is used to handle IPC messages for application-related
// requests like install/uninstall/launch etc.
// It works together with the application management system and act as an
// interface for outside requests.
class ApplicationDaemon {
  public:
    explicit ApplicationDaemon(xwalk::RuntimeContext* runtime_context);
    ~ApplicationDaemon();

    // Go into daemon mode, return false if any error occured, for example if
    // another daemon is already running in the background.
    bool Start();

  private:
    xwalk::RuntimeContext* runtime_context_;
    DISALLOW_COPY_AND_ASSIGN(ApplicationDaemon);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_DAEMON_H_
