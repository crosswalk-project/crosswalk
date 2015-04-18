// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "xwalk/application/common/manifest_handlers/user_agent_handler.h"

#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_manifest_keys;

namespace application {

UserAgentInfo::UserAgentInfo() {
}

UserAgentInfo::~UserAgentInfo() {
}

UserAgentHandler::UserAgentHandler() {
}

UserAgentHandler::~UserAgentHandler() {
}

bool UserAgentHandler::Parse(scoped_refptr<ApplicationData> application,
                             base::string16* error) {
  if (!application->GetManifest()->HasKey(keys::kXWalkUserAgentKey)) {
    application->SetManifestData(keys::kXWalkUserAgentKey, new UserAgentInfo);
    return true;
  }

  std::string user_agent;
  if (!application->GetManifest()->GetString(keys::kXWalkUserAgentKey,
                                             &user_agent) ||
      user_agent.empty()) {
    *error = base::ASCIIToUTF16("Invalid value of xwalk user agent.");
    return false;
  }

  scoped_ptr<UserAgentInfo> user_agent_info(new UserAgentInfo);
  user_agent_info->SetUserAgent(user_agent);
  application->SetManifestData(keys::kXWalkUserAgentKey,
                               user_agent_info.release());
  return true;
}

std::vector<std::string> UserAgentHandler::Keys() const {
  return std::vector<std::string>(1, keys::kXWalkUserAgentKey);
}

}  // namespace application
}  // namespace xwalk
