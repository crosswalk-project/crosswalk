// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_SETTING_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_SETTING_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "base/values.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class TizenSettingInfo : public ApplicationData::ManifestData {
 public:
  TizenSettingInfo();
  virtual ~TizenSettingInfo();

  void set_hwkey_enabled(bool enabled) { hwkey_enabled_ = enabled; }
  bool hwkey_enabled() const { return hwkey_enabled_; }

 private:
  bool hwkey_enabled_;
};

class TizenSettingHandler : public ManifestHandler {
 public:
  TizenSettingHandler();
  virtual ~TizenSettingHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) OVERRIDE;
  virtual bool Validate(scoped_refptr<const ApplicationData> application,
                        std::string* error,
                        std::vector<InstallWarning>* warnings) const OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenSettingHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_SETTING_HANDLER_H_
