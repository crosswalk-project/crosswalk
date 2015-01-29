// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_IME_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_IME_HANDLER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class TizenImeInfo : public ApplicationData::ManifestData {
 public:
  TizenImeInfo();
  ~TizenImeInfo() override;

  const std::string& uuid() const {
    return uuid_;
  }
  void set_uuid(const std::string& uuid) { uuid_ = uuid; }
  const std::vector<std::string>& languages() const {
    return languages_;
  }
  void AddLanguage(const std::string& language);

 private:
  std::string uuid_;
  std::vector<std::string> languages_;
};

class TizenImeHandler : public ManifestHandler {
 public:
  TizenImeHandler();
  ~TizenImeHandler() override;
  bool Parse(scoped_refptr<ApplicationData> application,
      base::string16* error) override;
  bool Validate(scoped_refptr<const ApplicationData> application,
      std::string* error) const override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenImeHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_IME_HANDLER_H_
