// Copyright (c 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/av_codecs_provider_ffmpeg.h"

#include <set>
#include <string>

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "content/public/common/content_paths.h"
#include "media/base/media.h"
#include "media/ffmpeg/ffmpeg_common.h"
#include "media/filters/ffmpeg_glue.h"

namespace xwalk {
namespace sysapps {

AVCodecsProviderFFmpeg::AVCodecsProviderFFmpeg() {
  media::InitializeMediaLibrary();
  media::FFmpegGlue::InitializeFFmpeg();
}

AVCodecsProviderFFmpeg::~AVCodecsProviderFFmpeg() {}

std::unique_ptr<SystemAVCodecs> AVCodecsProviderFFmpeg::GetSupportedCodecs() const {
  std::unique_ptr<SystemAVCodecs> av_codecs(new SystemAVCodecs);

  // Get a list of supported codecs.
  AVCodec* codec = NULL;
  while ((codec = av_codec_next(codec))) {
    if (codec->type == AVMEDIA_TYPE_AUDIO) {
      // Ensure the codec is supported by converting an FFmpeg audio codec ID
      // into its corresponding supported codec id.
      if (media::CodecIDToAudioCodec(codec->id)) {
        AudioCodec audio_codec;
        audio_codec.format = std::string(codec->name);
        av_codecs->audio_codecs.push_back(std::move(audio_codec));
      }
    } else if (codec->type == AVMEDIA_TYPE_VIDEO) {
      // Ensure the codec is supported by converting an FFmpeg video codec ID
      // into its corresponding supported codec id.
      if (media::CodecIDToVideoCodec(codec->id)) {
        VideoCodec video_codec;
        video_codec.format = std::string(codec->name);
        // FIXME(qjia7): find how to get hwAccel value
        video_codec.hw_accel = false;
        video_codec.encode = av_codec_is_encoder(codec) != 0;
        av_codecs->video_codecs.push_back(std::move(video_codec));
      }
    }
  }

  return av_codecs;
}

}  // namespace sysapps
}  // namespace xwalk
