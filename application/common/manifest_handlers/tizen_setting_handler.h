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

  enum ScreenOrientation {
    PORTRAIT,
    LANDSCAPE,
    AUTO
  };

  void set_hwkey_enabled(bool enabled) { hwkey_enabled_ = enabled; }
  bool hwkey_enabled() const { return hwkey_enabled_; }

  void set_screen_orientation(ScreenOrientation orientation) {
    screen_orientation_ = orientation;
  }

  ScreenOrientation screen_orientation() const { return screen_orientation_; }

  void set_encryption_enabled(bool enabled) { encryption_enabled_ = enabled; }
  bool encryption_enabled() const { return encryption_enabled_; }

  void set_context_menu_enabled(bool enabled) {
    context_menu_enabled_ = enabled;
  }
  bool context_menu_enabled() const { return context_menu_enabled_; }

  void set_background_support_enabled(bool enabled) {
    background_support_enabled_ = enabled;
  }
  bool background_support_enabled() const {
    return background_support_enabled_;
  }

 private:
  bool hwkey_enabled_;
  ScreenOrientation screen_orientation_;
  bool encryption_enabled_;
  bool context_menu_enabled_;
  bool background_support_enabled_;
};

class TizenSettingHandler : public ManifestHandler {
 public:
  TizenSettingHandler();
  virtual ~TizenSettingHandler();

  bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) override;
  bool Validate(scoped_refptr<const ApplicationData> application,
                        std::string* error) const override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenSettingHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_SETTING_HANDLER_H_
