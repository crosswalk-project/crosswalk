// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/csp_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_split.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {
namespace application {
namespace {
const char directive_separator = ';';
const char value_separator = ' ';
}  // namespace

CSPInfo::CSPInfo() {
}

CSPInfo::~CSPInfo() {
}

CSPHandler::CSPHandler(Manifest::PackageType type)
    : package_type_(type) {
}

CSPHandler::~CSPHandler() {
}

bool CSPHandler::Parse(scoped_refptr<ApplicationData> application,
                       base::string16* error) {
  if (package_type_ != application->GetPackageType())
    return false;
  scoped_ptr<CSPInfo> csp_info(new CSPInfo);
  std::string policies_str;
  const char* csp_key = GetCSPKey(package_type_);
  if (application->GetManifest()->HasKey(csp_key) &&
      !application->GetManifest()->GetString(csp_key, &policies_str)) {
    *error = base::ASCIIToUTF16("Invalid value of Content Security Policy (CSP).");
    return false;
  }

  std::vector<std::string> policies;
  base::SplitString(policies_str, directive_separator, &policies);
  for (size_t i = 0; i < policies.size(); ++i) {
    size_t found = policies[i].find(value_separator);
    if (found == std::string::npos) {
      *error = base::ASCIIToUTF16("Invalid value of directive: " + policies[i]);
      return false;
    }
    const std::string& directive_name = policies[i].substr(0, found);
    const std::string& directive_value_str = policies[i].substr(found+1);
    std::vector<std::string> directive_value;
    base::SplitStringAlongWhitespace(directive_value_str, &directive_value);
    csp_info->SetDirective(directive_name, directive_value);
  }
  application->SetManifestData(csp_key, csp_info.release());

  return true;
}

bool CSPHandler::AlwaysParseForType(Manifest::Type type) const {
  return package_type_ == Manifest::TYPE_XPK;
}

std::vector<std::string> CSPHandler::Keys() const {
  return std::vector<std::string>(1, GetCSPKey(package_type_));
}

}  // namespace application
}  // namespace xwalk
