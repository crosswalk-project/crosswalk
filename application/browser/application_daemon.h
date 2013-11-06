// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_DAEMON_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_DAEMON_H_

#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {
namespace application {

class ApplicationDaemon {
  public:
    explicit ApplicationDaemon(xwalk::RuntimeContext* runtime_context);
    ~ApplicationDaemon();

    // Go into daemon mode, return false if any error occured, for example if
    // another daemon is already running in the background.
    bool Daemonize();

  private:
    DISALLOW_COPY_AND_ASSIGN(ApplicationDaemon);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_DAEMON_H_
