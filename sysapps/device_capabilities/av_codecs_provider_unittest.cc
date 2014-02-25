// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/av_codecs_provider.h"

#include <vector>

#include "media/base/media.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/sysapps/common/sysapps_manager.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

using xwalk::jsapi::device_capabilities::AudioCodec;
using xwalk::jsapi::device_capabilities::VideoCodec;
using xwalk::jsapi::device_capabilities::SystemAVCodecs;
using xwalk::sysapps::AVCodecsProvider;

TEST(XWalkSysAppsDeviceCapabilitiesTest, AVCodecsProvider) {
  media::InitializeMediaLibraryForTesting();
  xwalk::sysapps::SysAppsManager manager;

  AVCodecsProvider* provider(manager.GetAVCodecsProvider());
  EXPECT_TRUE(provider != NULL);

  scoped_ptr<SystemAVCodecs> info(provider->GetSupportedCodecs());
  std::vector<linked_ptr<AudioCodec> > audio_codecs = info->audio_codecs;
  for (size_t i = 0; i < audio_codecs.size(); ++i)
    EXPECT_FALSE(audio_codecs[i]->format.empty());

  std::vector<linked_ptr<VideoCodec> > video_codecs = info->video_codecs;
  for (size_t i = 0; i < video_codecs.size(); ++i)
    EXPECT_FALSE(video_codecs[i]->format.empty());
}
