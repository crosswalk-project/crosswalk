// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/warp_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

WARPInfo::WARPInfo() {
}

WARPInfo::~WARPInfo() {
}

WARPHandler::WARPHandler() {
}

WARPHandler::~WARPHandler() {
}

bool WARPHandler::Parse(scoped_refptr<ApplicationData> application,
                       base::string16* error) {
  base::Value* value;
  if (!application->GetManifest()->HasPath(keys::kAccessKey))
    return true;
  if (!application->GetManifest()->Get(keys::kAccessKey, &value)) {
    *error = base::ASCIIToUTF16(
        "Invalid value of Widget Access Request Policy(WARP).");
    return false;
  }

  scoped_ptr<base::ListValue> warp_list;
  if (value->IsType(base::Value::TYPE_DICTIONARY)) {
    warp_list.reset(new base::ListValue);
    warp_list->Append(value->DeepCopy());
  } else {
    base::ListValue* list;
    value->GetAsList(&list);
    if (list)
      warp_list.reset(list->DeepCopy());
  }

  if (!warp_list) {
    *error = base::ASCIIToUTF16(
        "Invalid Widget Access Request Policy(WARP) list.");
    return false;
  }

  scoped_ptr<WARPInfo> warp_info(new WARPInfo);
  warp_info->SetWARP(warp_list.release());
  application->SetManifestData(keys::kAccessKey, warp_info.release());

  return true;
}

std::vector<std::string> WARPHandler::Keys() const {
  return std::vector<std::string>(1, keys::kAccessKey);
}

}  // namespace application
}  // namespace xwalk
