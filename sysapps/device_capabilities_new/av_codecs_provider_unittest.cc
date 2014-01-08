// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities_new/av_codecs_provider.h"

#include <vector>

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "content/public/common/content_paths.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xwalk/sysapps/common/sysapps_manager.h"
#include "xwalk/sysapps/device_capabilities_new/device_capabilities.h"

using xwalk::jsapi::device_capabilities::AudioCodec;
using xwalk::jsapi::device_capabilities::VideoCodec;
using xwalk::jsapi::device_capabilities::SystemAVCodecs;
using xwalk::sysapps::AVCodecsProvider;

TEST(XWalkSysAppsDeviceCapabilitiesTest, AVCodecsProvider) {
  // Ensure the content::RegisterPathProvider() is called, but
  // test first to make sure it is not registered twice. The ffmpeg
  // codec provider needs it.
  base::FilePath media_path;
  PathService::Get(content::DIR_MEDIA_LIBS, &media_path);
  if (media_path.empty())
    content::RegisterPathProvider();

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
