// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_NAVIGATION_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_NAVIGATION_HANDLER_H_

#include <string>
#include <vector>

#include "base/strings/string_split.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class TizenNavigationInfo : public ApplicationData::ManifestData {
 public:
  explicit TizenNavigationInfo(const std::string& allowed_domains);
  virtual ~TizenNavigationInfo();

  const std::vector<std::string>& GetAllowedDomains() const {
    return allowed_domains_;
  }

 private:
  std::vector<std::string> allowed_domains_;
};

class TizenNavigationHandler : public ManifestHandler {
 public:
  TizenNavigationHandler();
  virtual ~TizenNavigationHandler();

  bool Parse(scoped_refptr<ApplicationData> application_data,
             base::string16* error) override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenNavigationHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_NAVIGATION_HANDLER_H_
