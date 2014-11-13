// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_permissions_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/permissions_handler.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

TizenPermissionsHandler::TizenPermissionsHandler() {
}

TizenPermissionsHandler::~TizenPermissionsHandler() {
}

bool TizenPermissionsHandler::Parse(scoped_refptr<ApplicationData> application,
                                    base::string16* error) {
  if (!application->GetManifest()->HasPath(keys::kTizenPermissionsKey)) {
    application->SetManifestData(
        keys::kTizenPermissionsKey, new PermissionsInfo);
    return true;
  }

  base::Value* value;
  if (!application->GetManifest()->Get(keys::kTizenPermissionsKey, &value)) {
    *error = base::ASCIIToUTF16("Invalid value of tizen permissions.");
    return false;
  }

  scoped_ptr<base::ListValue> permission_list;
  if (value->IsType(base::Value::TYPE_DICTIONARY)) {
    permission_list.reset(new base::ListValue);
    permission_list->Append(value->DeepCopy());
  } else {
    base::ListValue* list = NULL;
    value->GetAsList(&list);
    if (list)
      permission_list.reset(list->DeepCopy());
  }

  if (!permission_list) {
    *error = base::ASCIIToUTF16("Invalid value of permissions.");
    return false;
  }

  scoped_ptr<PermissionsInfo> permissions_info(new PermissionsInfo);
  PermissionSet api_permissions;
  for (base::ListValue::const_iterator it = permission_list->begin();
       it != permission_list->end(); ++it) {
    base::DictionaryValue* dictionary_value = NULL;
    (*it)->GetAsDictionary(&dictionary_value);

    std::string permission;
    if (!dictionary_value ||
        !dictionary_value->GetString(
            keys::kTizenPermissionsNameKey, &permission) ||
        permission.empty())
      continue;

    if (api_permissions.find(permission) != api_permissions.end())
      LOG(WARNING) << "Duplicated permission names found.";

    api_permissions.insert(permission);
  }

  permissions_info->SetAPIPermissions(api_permissions);
  application->SetManifestData(keys::kTizenPermissionsKey,
                               permissions_info.release());

  return true;
}

bool TizenPermissionsHandler::AlwaysParseForType(Manifest::Type type) const {
  return type == Manifest::TYPE_WIDGET;
}

std::vector<std::string> TizenPermissionsHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenPermissionsKey);
}

}  // namespace application
}  // namespace xwalk
