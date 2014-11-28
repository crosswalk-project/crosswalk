// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"

class GURL;

namespace base {
class CommandLine;
}

namespace content {
class RenderProcessHost;
}

namespace xwalk {
class XWalkBrowserContext;
}

namespace xwalk {
namespace application {

class ApplicationService;

// The ApplicationSystem manages the creation and destruction of services which
// related to applications' runtime model.
// There's one-to-one correspondence between ApplicationSystem and
// XWalkBrowserContext.
class ApplicationSystem {
 public:
  virtual ~ApplicationSystem();

  static scoped_ptr<ApplicationSystem> Create(
      XWalkBrowserContext* browser_context);

  // The ApplicationService is created at startup.
  ApplicationService* application_service() {
    return application_service_.get();
  }

  // Launches an application based on the given command line, there are
  // different ways to inform which application should be launched
  //
  // (1) app_id from the binary name (used in Tizen);
  // (2) app_id passed in the command line (used in Tizen);
  // (3) launching a directory that contains an extracted package.
  // (4) launching from the path to the packaged application file.
  //
  // The parameter `url` contains the current URL Crosswalk is considering to
  // load, and the output parameter `run_default_message_loop` controls whether
  // Crosswalk should run the mainloop or not.
  //
  // A return value of true indicates that ApplicationSystem handled the command
  // line, so the caller shouldn't try to load the url by itself.
  bool LaunchFromCommandLine(const base::CommandLine& cmd_line,
                             const GURL& url,
                             bool& run_default_message_loop_);  // NOLINT

  void CreateExtensions(content::RenderProcessHost* host,
                        extensions::XWalkExtensionVector* extensions);

 protected:
  explicit ApplicationSystem(XWalkBrowserContext* browser_context);

 private:
  // Note: initialization order matters.
  XWalkBrowserContext* browser_context_;
  scoped_ptr<ApplicationService> application_service_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationSystem);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_
