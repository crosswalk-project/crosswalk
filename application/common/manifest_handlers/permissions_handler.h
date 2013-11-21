// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_PERMISSIONS_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_PERMISSIONS_HANDLER_H_

#include <string>
#include <vector>

#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class PermissionsInfo: public Application::ManifestData {
 public:
  PermissionsInfo();
  virtual ~PermissionsInfo();

  const std::vector<std::string>& GetAPIPermissions() const {
    return api_permissions_;}
  void SetAPIPermissions(const std::vector<std::string>& api_permissions) {
    api_permissions_ = api_permissions;
  }

 private:
  std::vector<std::string> api_permissions_;
  DISALLOW_COPY_AND_ASSIGN(PermissionsInfo);
};

class PermissionsHandler: public ManifestHandler {
 public:
  PermissionsHandler();
  virtual ~PermissionsHandler();

  virtual bool Parse(scoped_refptr<Application> application,
                     string16* error) OVERRIDE;
  virtual bool AlwaysParseForType(Manifest::Type type) const OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(PermissionsHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_PERMISSIONS_HANDLER_H_
