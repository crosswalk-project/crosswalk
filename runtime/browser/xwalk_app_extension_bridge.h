// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_APP_EXTENSION_BRIDGE_H_
#define XWALK_RUNTIME_BROWSER_XWALK_APP_EXTENSION_BRIDGE_H_

#include <string>

#include "xwalk/application/browser/application_system.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_permission_types.h"

namespace xwalk {

// The class is the main interface for co-operations between the two major
// xwalk components -- the application subsystem and the extension subsystem.
// Because of the dependency requirements two components cound not include
// each other's header, and we also would not like to fill the xwalk_runner
// with delegates, so this class is the place where all the communications
// between application and extension takes place, just like a 'bridge'.
// The class instance will be owned by xwalk_runner.
class XWalkAppExtensionBridge
    : public extensions::XWalkExtensionService::Delegate {
 public:
  XWalkAppExtensionBridge();
  virtual ~XWalkAppExtensionBridge();

  void SetApplicationSystem(application::ApplicationSystem* app_system) {
    app_system_ = app_system;
  }
  // XWalkExtensionService::Delegate implementation
  virtual void CheckAPIAccessControl(
      int render_process_id,
      const std::string& extension_name,
      const std::string& api_name,
      const extensions::PermissionCallback& callback) OVERRIDE;
  virtual bool RegisterPermissions(
      int render_process_id,
      const std::string& extension_name,
      const std::string& perm_table) OVERRIDE;

 private:
  application::ApplicationSystem* app_system_;

  DISALLOW_COPY_AND_ASSIGN(XWalkAppExtensionBridge);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_APP_EXTENSION_BRIDGE_H_
