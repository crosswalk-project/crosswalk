// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APP_CONTROL_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APP_CONTROL_HANDLER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

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

 private:
  std::string src_;
  std::string operation_;
  std::string uri_;
  std::string mime_;
};

struct AppControlInfoList : public ApplicationData::ManifestData {
  std::vector<AppControlInfo> controls;
};

class TizenAppControlHandler : public ManifestHandler {
 public:
  TizenAppControlHandler();
  virtual ~TizenAppControlHandler();
  bool Parse(scoped_refptr<ApplicationData> application,
      base::string16* error) override;
  bool Validate(scoped_refptr<const ApplicationData> application,
      std::string* error) const override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenAppControlHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APP_CONTROL_HANDLER_H_
