// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_AV_CODECS_PROVIDER_ANDROID_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_AV_CODECS_PROVIDER_ANDROID_H_

#include "xwalk/sysapps/device_capabilities/av_codecs_provider.h"

namespace xwalk {
namespace sysapps {

class AVCodecsProviderAndroid : public AVCodecsProvider {
 public:
  AVCodecsProviderAndroid();
  virtual ~AVCodecsProviderAndroid();

  virtual scoped_ptr<SystemAVCodecs> GetSupportedCodecs() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(AVCodecsProviderAndroid);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_AV_CODECS_PROVIDER_ANDROID_H_
