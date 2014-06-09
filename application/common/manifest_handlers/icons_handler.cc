// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/icons_handler.h"

#include <algorithm>
#include <vector>

#include "base/file_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace {

bool ParseWidgetIconItem(base::DictionaryValue* dict,
                         uint32* size,
                         base::FilePath* src) {
  std::string width_string;
  std::string height_string;
  std::string src_string;

  dict->GetString(keys::kIconWidthKey, &width_string);
  dict->GetString(keys::kIconHeightKey, &height_string);
  if (!dict->GetString(keys::kIconSrcKey, &src_string))
    return false;

  uint32 width, height;
  base::StringToUint(width_string, &width);
  base::StringToUint(height_string, &height);

  *size = std::max(width, height);
  *src = base::FilePath(src_string);
  return true;
}

inline bool CheckIconPath(const base::FilePath& app_path,
                          const base::FilePath& src) {
  return ((!src.IsAbsolute()) && base::PathExists(app_path.Append(src)));
}

}  // namespace

namespace application {

IconsInfo::IconsInfo() {
}

IconsInfo::~IconsInfo() {
}

void IconsInfo::SetIcon(uint32 size, const base::FilePath& src) {
  if (icons_.find(size) == icons_.end())
    icons_[size] = src;
}

IconsHandler::IconsHandler(Package::Type type)
    : package_type_(type) {
}

IconsHandler::~IconsHandler() {
}

bool IconsHandler::Parse(scoped_refptr<ApplicationData> application,
                          base::string16* error) {
  scoped_ptr<IconsInfo> icons_info(new IconsInfo);
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  if (application->GetPackageType() == Package::WGT) {
    base::Value* value = NULL;
    manifest->Get(GetIconKey(package_type_), &value);

    base::DictionaryValue* dict;
    base::ListValue* list;

    if (value && value->GetAsDictionary(&dict)) {
      uint32 size;
      base::FilePath src;
      if (ParseWidgetIconItem(dict, &size, &src)
         && CheckIconPath(application->Path(), src))
        icons_info->SetIcon(size, src);
    } else if (value && value->GetAsList(&list)) {
      for (base::ListValue::iterator it = list->begin();
         it != list->end(); ++it) {
        (*it)->GetAsDictionary(&dict);
        uint32 size;
        base::FilePath src;
        if (ParseWidgetIconItem(dict, &size, &src)
           && CheckIconPath(application->Path(), src))
          icons_info->SetIcon(size, src);
      }
    }
  } else {
    base::Value* value = NULL;
    base::DictionaryValue* dict;
    if (manifest->Get(GetIconKey(package_type_), &value)
       && value->GetAsDictionary(&dict)) {
      base::DictionaryValue::Iterator it(*dict);
      while (!it.IsAtEnd()) {
        uint32 size;
        std::string src_string;
        base::FilePath src;
        base::StringToUint(it.key(), &size);
        it.value().GetAsString(&src_string);
        src = base::FilePath(src_string);
        if (CheckIconPath(application->Path(), src)) {
          icons_info->SetIcon(size, src);
        } else {
          *error = base::ASCIIToUTF16("Cannot find icon at path.");
          return false;
        }
        it.Advance();
      }
    }
  }
  application->SetManifestData(GetIconKey(package_type_), icons_info.release());
  return true;
}

std::vector<std::string> IconsHandler::Keys() const {
  return std::vector<std::string>(1, GetIconKey(package_type_));
}

}  // namespace application
}  // namespace xwalk
