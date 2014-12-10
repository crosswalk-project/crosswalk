// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_SPLASH_SCREEN_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_SPLASH_SCREEN_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "base/values.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class TizenSplashScreenInfo : public ApplicationData::ManifestData {
 public:
  TizenSplashScreenInfo();
  virtual ~TizenSplashScreenInfo();

  void set_src(const std::string &src) { src_ = src; }
  const std::string& src() const { return src_; }

 private:
  std::string src_;
};

class TizenSplashScreenHandler : public ManifestHandler {
 public:
  TizenSplashScreenHandler();
  virtual ~TizenSplashScreenHandler();

  bool Parse(scoped_refptr<ApplicationData> application,
             base::string16* error) override;
  bool Validate(scoped_refptr<const ApplicationData> application,
                std::string* error) const override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenSplashScreenHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_SPLASH_SCREEN_HANDLER_H_
