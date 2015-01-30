// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_TIZEN_H_

#include "base/command_line.h"

#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_tizen.h"

namespace xwalk {
namespace application {

class ApplicationSystemTizen : public ApplicationSystem {
 public:
  explicit ApplicationSystemTizen(XWalkBrowserContext* browser_context);
  ~ApplicationSystemTizen() override;
  virtual bool LaunchFromCommandLine(const base::CommandLine& cmd_line,
                                     const GURL& url) override;
 private:
  DISALLOW_COPY_AND_ASSIGN(ApplicationSystemTizen);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_TIZEN_H_
