// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/permissions_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

PermissionsInfo::PermissionsInfo() {
}

PermissionsInfo::~PermissionsInfo() {
}

PermissionsHandler::PermissionsHandler() {
}

PermissionsHandler::~PermissionsHandler() {
}

bool PermissionsHandler::Parse(scoped_refptr<ApplicationData> application,
                               string16* error) {
  if (!application->GetManifest()->HasKey(keys::kPermissionsKey)) {
    application->SetManifestData(keys::kPermissionsKey, new PermissionsInfo);
    return true;
  }

  const base::ListValue* permissions = NULL;
  if (!application->GetManifest()->GetList(
          keys::kPermissionsKey, &permissions) || !permissions) {
    *error = ASCIIToUTF16("Invalid value of permissions.");
    return false;
  }

  scoped_ptr<PermissionsInfo> permissions_info(new PermissionsInfo);
  PermissionSet api_permissions;
  for (size_t i = 0; i < permissions->GetSize(); ++i) {
    std::string permission;
    if (!permissions->GetString(i, &permission)) {
      *error = ASCIIToUTF16(
          "An error occurred when parsing permission string.");
      return false;
    }
    if (api_permissions.find(permission) != api_permissions.end()) {
      *error = ASCIIToUTF16(
          "Duplicated permission names found.");
      return false;
    }
    api_permissions.insert(permission);
  }
  permissions_info->SetAPIPermissions(api_permissions);
  application->SetManifestData(keys::kPermissionsKey,
                               permissions_info.release());

  return true;
}

bool PermissionsHandler::AlwaysParseForType(Manifest::Type type) const {
  return true;
}

std::vector<std::string> PermissionsHandler::Keys() const {
  return std::vector<std::string>(1, keys::kPermissionsKey);
}

}  // namespace application
}  // namespace xwalk
