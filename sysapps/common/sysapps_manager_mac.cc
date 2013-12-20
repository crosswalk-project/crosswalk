// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/sysapps_manager.h"

#include "base/basictypes.h"
#include "xwalk/sysapps/device_capabilities_new/av_codecs_provider_ffmpeg.h"

namespace xwalk {
namespace sysapps {

// static
AVCodecsProvider* SysAppsManager::GetAVCodecsProvider() {
  CR_DEFINE_STATIC_LOCAL(AVCodecsProviderFFmpeg, provider, ());

  return &provider;
}

}  // namespace sysapps
}  // namespace xwalk
