// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_APP_CONTROL_INFO_H_
#define XWALK_APPLICATION_COMMON_TIZEN_APP_CONTROL_INFO_H_

#include <memory>
#include <string>
#include <vector>

#include "xwalk/application/common/application_data.h"

namespace xwalk {
namespace application {

class AppControlInfo {
 public:
  AppControlInfo(const std::string& src, const std::string& operation,
      const std::string& uri, const std::string& mime)
      : src_(src),
        operation_(operation),
        uri_(uri),
        mime_(mime) { }
  const std::string& src() const {
    return src_;
  }
  const std::string& operation() const {
    return operation_;
  }
  const std::string& uri() const {
    return uri_;
  }
  const std::string& mime() const {
    return mime_;
  }
  bool Covers(const AppControlInfo& requested) const;

  static std::unique_ptr<AppControlInfo> CreateFromBundle(
      const std::string& bundle_str);

 private:
  std::string src_;
  std::string operation_;
  std::string uri_;
  std::string mime_;
};

struct AppControlInfoList : public ApplicationData::ManifestData {
  std::vector<AppControlInfo> controls;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_APP_CONTROL_INFO_H_
