// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

class CommandLine;
class GURL;

namespace xwalk {
class RuntimeContext;
}

namespace xwalk {
namespace application {

class ApplicationEventManager;
class ApplicationProcessManager;
class ApplicationService;
class ApplicationServiceProvider;

// The ApplicationSystem manages the creation and destruction of services which
// related to applications' runtime model.
// There's one-to-one correspondence between ApplicationSystem and
// RuntimeContext.
class ApplicationSystem {
 public:
  explicit ApplicationSystem(xwalk::RuntimeContext* runtime_context);
  ~ApplicationSystem();

  // The ApplicationProcessManager is created at startup.
  ApplicationProcessManager* process_manager() {
    return process_manager_.get();
  }

  // The ApplicationService is created at startup.
  ApplicationService* application_service() {
    return application_service_.get();
  }

  // The ApplicationEventManager is created at startup.
  ApplicationEventManager* event_manager() {
    return event_manager_.get();
  }

  // Parse the command line and process the --install, --uninstall and
  // --list-apps commands. Returns true when a management command was processed,
  // so the caller shouldn't load a runtime.
  //
  // The parameter `url` contains the current URL Crosswalk is considering to
  // load.
  bool HandleApplicationManagementCommands(const CommandLine& cmd_line,
                                           const GURL& url);

  // Launches an application based on the given command line, there are
  // different ways to inform which application should be launched
  //
  // (1) app_id from the binary name (used in Tizen);
  // (2) app_id passed in the command line;
  // (3) launching a directory that contains an extracted package.
  //
  // The parameter `url` contains the current URL Crosswalk is considering to
  // load, and the output parameter `run_default_message_loop` controls whether
  // Crosswalk should run the mainloop or not.
  //
  // A return value of true indicates that ApplicationSystem handled the command
  // line, so the caller shouldn't try to load the url by itself.
  bool LaunchFromCommandLine(const CommandLine& cmd_line, const GURL& url,
                             bool* run_default_message_loop_);

  bool is_running_as_service() const { return !!service_provider_.get(); }

 private:
  xwalk::RuntimeContext* runtime_context_;
  scoped_ptr<ApplicationProcessManager> process_manager_;
  scoped_ptr<ApplicationService> application_service_;
  scoped_ptr<ApplicationEventManager> event_manager_;
  scoped_ptr<ApplicationServiceProvider> service_provider_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationSystem);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_
