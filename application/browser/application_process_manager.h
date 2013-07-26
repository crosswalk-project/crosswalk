// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_

#include <map>
#include <set>
#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/time.h"
#include "xwalk/application/common/application.h"

class GURL;

namespace xwalk {
class Runtime;
class RuntimeContext;
}

namespace xwalk_application {

class Application;
class ApplicationHost;

// This manages dynamic state of running applications. By now, it only launches
// one application, later it will manages all event pages' lifecycle.
class ApplicationProcessManager {
 public:
  explicit ApplicationProcessManager(xwalk::RuntimeContext* runtime_context);
  ~ApplicationProcessManager();

  void LaunchApplication(xwalk::RuntimeContext* runtime_context,
                       const Application* application);

 private:
  base::WeakPtrFactory<ApplicationProcessManager> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationProcessManager);
};

}  // namespace xwalk_application

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_PROCESS_MANAGER_H_
