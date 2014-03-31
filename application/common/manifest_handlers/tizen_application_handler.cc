// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"

#include <map>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_split.h"
#include "third_party/re2/re2/re2.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

TizenApplicationInfo::TizenApplicationInfo() {
}

TizenApplicationInfo::~TizenApplicationInfo() {
}

TizenApplicationHandler::TizenApplicationHandler() {}

TizenApplicationHandler::~TizenApplicationHandler() {}

bool TizenApplicationHandler::Parse(scoped_refptr<ApplicationData> application,
                                    base::string16* error) {
  scoped_ptr<TizenApplicationInfo> app_info(new TizenApplicationInfo);
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  base::Value* app_value = NULL;
  manifest->Get(keys::kTizenApplicationKey, &app_value);
  // Find an application element with tizen namespace
  base::DictionaryValue* app_dict;
  std::string value;
  bool find = false;
  if (app_value && app_value->IsType(base::Value::TYPE_DICTIONARY)) {
    app_value->GetAsDictionary(&app_dict);
    find = app_dict->GetString(keys::kNamespaceKey, &value);
    find = find && (value == kTizenNamespacePrefix);
  } else if (app_value && app_value->IsType(base::Value::TYPE_LIST)) {
    base::ListValue* list;
    app_value->GetAsList(&list);
    for (base::ListValue::iterator it = list->begin();
         it != list->end(); ++it) {
      (*it)->GetAsDictionary(&app_dict);
      find = app_dict->GetString(keys::kNamespaceKey, &value);
      find = find && (value == kTizenNamespacePrefix);
      if (find)
        break;
    }
  }

  if (!find) {
    *error = base::ASCIIToUTF16(
        "Cannot find application element with tizen namespace"
        " or the tizen namespace prefix is incorrect.\n");
    return false;
  }
  if (app_dict->GetString(keys::kTizenApplicationIdKey, &value))
    app_info->set_id(value);
  if (app_dict->GetString(keys::kTizenApplicationPackageKey, &value))
    app_info->set_package(value);
  if (app_dict->GetString(keys::kTizenApplicationRequiredVersionKey, &value))
    app_info->set_required_version(value);

  application->SetManifestData(keys::kTizenApplicationKey,
                               app_info.release());
  return true;
}

bool TizenApplicationHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error,
    std::vector<InstallWarning>* warnings) const {
  const TizenApplicationInfo* app_info =
      static_cast<const TizenApplicationInfo*>(
          application->GetManifestData(keys::kTizenApplicationKey));

  const char kIdPattern[] = "\\A[0-9a-zA-Z]{10}[.][0-9a-zA-Z]{1,52}\\z";
  const char kPackagePattern[] = "\\A[0-9a-zA-Z]{10}\\z";
  if (!RE2::PartialMatch(app_info->id(), kIdPattern)) {
    *error = std::string("The id property of application element"
                         " does not match the format\n");
    return false;
  }
  if (!RE2::PartialMatch(app_info->package(), kPackagePattern)) {
    *error = std::string("The package property of application element"
                         " does not match the format\n");
    return false;
  }
  if (app_info->id().find(app_info->package()) != 0) {
    *error = std::string("The application element property id"
                         " does not start with package.\n");
    return false;
  }
  // TODO(hongzhang): We need a version map (Tizen API version
  // to Crosswalk API version) for checking required_version
  if (app_info->required_version().empty()) {
    *error = std::string("The required_version property of application"
                         " element does not exist.\n");
    return false;
  }

  return true;
}

std::vector<std::string> TizenApplicationHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenApplicationKey);
}

}  // namespace application
}  // namespace xwalk
