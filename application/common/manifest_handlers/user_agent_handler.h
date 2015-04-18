// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_USER_AGENT_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_USER_AGENT_HANDLER_H_

#include <string>
#include <vector>

#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"
namespace xwalk {
namespace application {

class UserAgentInfo : public ApplicationData::ManifestData {
 public:
  UserAgentInfo();
  ~UserAgentInfo() override;

  void SetUserAgent(const std::string& user_agent) {
    user_agent_ = user_agent;
  }

  const std::string& UserAgent() const {
    return user_agent_;
  }

 private:
  std::string user_agent_;
};

class UserAgentHandler : public ManifestHandler {
 public:
  UserAgentHandler();
  ~UserAgentHandler() override;

  bool Parse(scoped_refptr<ApplicationData> application,
             base::string16* error) override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(UserAgentHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_USER_AGENT_HANDLER_H_
