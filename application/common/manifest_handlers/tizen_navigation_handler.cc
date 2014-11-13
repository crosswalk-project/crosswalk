// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_navigation_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {
namespace {

const char navigation_separator = ' ';

}  // namespace

TizenNavigationInfo::TizenNavigationInfo(const std::string& allowed_domains) {
  base::SplitString(allowed_domains, navigation_separator, &allowed_domains_);
}

TizenNavigationInfo::~TizenNavigationInfo() {
}

TizenNavigationHandler::TizenNavigationHandler() {
}

TizenNavigationHandler::~TizenNavigationHandler() {
}

bool TizenNavigationHandler::Parse(
    scoped_refptr<ApplicationData> application_data,
    base::string16* error) {
  if (!application_data->GetManifest()->HasPath(keys::kAllowNavigationKey))
    return true;
  std::string allowed_domains;
  if (!application_data->GetManifest()->GetString(keys::kAllowNavigationKey,
                                                  &allowed_domains)) {
    *error = base::ASCIIToUTF16("Invalid value of allow-navigation.");
    return false;
  }
  if (allowed_domains.empty())
    return true;

  application_data->SetManifestData(keys::kAllowNavigationKey,
                                    new TizenNavigationInfo(allowed_domains));

  return true;
}

std::vector<std::string> TizenNavigationHandler::Keys() const {
  return std::vector<std::string>(1, keys::kAllowNavigationKey);
}

}  // namespace application
}  // namespace xwalk
