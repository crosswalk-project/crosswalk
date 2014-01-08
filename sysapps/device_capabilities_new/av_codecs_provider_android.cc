// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/av_codecs_provider_android.h"

namespace xwalk {
namespace sysapps {

AVCodecsProviderAndroid::AVCodecsProviderAndroid() {}

AVCodecsProviderAndroid::~AVCodecsProviderAndroid() {}

scoped_ptr<SystemAVCodecs> AVCodecsProviderAndroid::GetSupportedCodecs() const {
  NOTIMPLEMENTED();
  return make_scoped_ptr(new SystemAVCodecs);
}

}  // namespace sysapps
}  // namespace xwalk
