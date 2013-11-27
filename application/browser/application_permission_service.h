// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_PERMISSION_SERVICE_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_PERMISSION_SERVICE_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "xwalk/application/common/application.h"
#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {
class RuntimeContext;
namespace application {
// This will manages application api permission control
class ApplicationPermissionService {
 public:
  explicit ApplicationPermissionService(xwalk::RuntimeContext* runtime_context);
  virtual ~ApplicationPermissionService();

  static bool CheckAPIAccessControl(std::string extension_name,
      std::string app_id, std::string api_name);

 private:
  xwalk::RuntimeContext* runtime_context_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationPermissionService);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_PERMISSION_SERVICE_H_
