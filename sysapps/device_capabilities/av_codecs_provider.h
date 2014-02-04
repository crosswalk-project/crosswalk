// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_AV_CODECS_PROVIDER_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_AV_CODECS_PROVIDER_H_

#include "base/memory/scoped_ptr.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

namespace xwalk {
namespace sysapps {

using jsapi::device_capabilities::AudioCodec;
using jsapi::device_capabilities::VideoCodec;
using jsapi::device_capabilities::SystemAVCodecs;

class AVCodecsProvider {
 public:
  AVCodecsProvider() {}
  virtual ~AVCodecsProvider() {}

  virtual scoped_ptr<SystemAVCodecs> GetSupportedCodecs() const = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(AVCodecsProvider);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_AV_CODECS_PROVIDER_H_
