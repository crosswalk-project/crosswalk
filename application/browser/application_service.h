// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_

#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {
class RuntimeContext;
}

namespace xwalk_application {

// This will manages applications install, uninstall, update and so on. It'll
// also maintain all installed applications' info.
class ApplicationService {
 public:
  explicit ApplicationService(xwalk::RuntimeContext* runtime_context);
  virtual ~ApplicationService();

 private:
  xwalk::RuntimeContext* runtime_context_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationService);
};

}  // namespace xwalk_application

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_H_
