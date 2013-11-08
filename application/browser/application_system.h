// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/application/browser/application_service_provider.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_service.h"

namespace xwalk {
class RuntimeContext;
}

namespace xwalk {
namespace application {

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

  // Create the service object together with application system.
  ApplicationServiceProvider* application_service_provider() {
    return application_service_provider_.get();
  }

 private:
  xwalk::RuntimeContext* runtime_context_;
  scoped_ptr<ApplicationProcessManager> process_manager_;
  scoped_ptr<ApplicationService> application_service_;
  scoped_ptr<ApplicationServiceProvider> application_service_provider_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationSystem);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SYSTEM_H_
