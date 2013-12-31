// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/csp_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_split.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

CSPInfo::CSPInfo() {
}

CSPInfo::~CSPInfo() {
}

CSPHandler::CSPHandler() {
}

CSPHandler::~CSPHandler() {
}

bool CSPHandler::Parse(scoped_refptr<ApplicationData> application,
                       string16* error) {
  scoped_ptr<CSPInfo> csp_info(new CSPInfo);
  std::string policies_str;
  if (application->GetManifest()->HasKey(keys::kCSPKey) &&
      !application->GetManifest()->GetString(keys::kCSPKey, &policies_str)) {
    *error = ASCIIToUTF16("Invalid value of Content Security Policy (CSP).");
    return false;
  }

  std::vector<std::string> policies;
  base::SplitString(policies_str, ';', &policies);
  for (size_t i = 0; i < policies.size(); ++i) {
    size_t found = policies[i].find(' ');
    if (found == std::string::npos) {
      *error = ASCIIToUTF16("Invalid value of directive: " + policies[i]);
      return false;
    }
    const std::string& directive_name = policies[i].substr(0, found);
    const std::string& directive_value_str = policies[i].substr(found+1);
    std::vector<std::string> directive_value;
    base::SplitStringAlongWhitespace(directive_value_str, &directive_value);
    csp_info->SetDirective(directive_name, directive_value);
  }
  application->SetManifestData(keys::kCSPKey, csp_info.release());

  return true;
}

bool CSPHandler::AlwaysParseForType(Manifest::Type type) const {
  return true;
}

std::vector<std::string> CSPHandler::Keys() const {
  return std::vector<std::string>(1, keys::kCSPKey);
}

}  // namespace application
}  // namespace xwalk
