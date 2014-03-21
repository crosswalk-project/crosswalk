// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_NAVIGATION_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_NAVIGATION_HANDLER_H_

#include <string>
#include <vector>

#include "base/strings/string_split.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class NavigationInfo : public ApplicationData::ManifestData {
 public:
  explicit NavigationInfo(const std::string& allowed_domains);
  virtual ~NavigationInfo();

  const std::vector<std::string>& GetAllowedDomains() const {
    return allowed_domains_; }

 private:
  std::vector<std::string> allowed_domains_;
};

class NavigationHandler : public ManifestHandler {
 public:
  NavigationHandler();
  virtual ~NavigationHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application_data,
                     base::string16* error) OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(NavigationHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_NAVIGATION_HANDLER_H_
