// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/av_codecs_provider_android.h"

namespace xwalk {
namespace sysapps {

AVCodecsProviderAndroid::AVCodecsProviderAndroid() {}

AVCodecsProviderAndroid::~AVCodecsProviderAndroid() {}

std::unique_ptr<SystemAVCodecs> AVCodecsProviderAndroid::GetSupportedCodecs() const {
  NOTIMPLEMENTED();
  return base::WrapUnique(new SystemAVCodecs);
}

}  // namespace sysapps
}  // namespace xwalk
