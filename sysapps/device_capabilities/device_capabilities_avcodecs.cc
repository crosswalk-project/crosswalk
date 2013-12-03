// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/device_capabilities/device_capabilities_avcodecs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}  // extern "C"

#include "base/files/file_path.h"
#include "base/strings/string_split.h"
#include "base/path_service.h"
#include "content/public/common/content_paths.h"
#include "media/filters/ffmpeg_glue.h"
#include "media/base/media.h"
#include "net/base/mime_util.h"

namespace xwalk {
namespace sysapps {

DeviceCapabilitiesAVCodecs::DeviceCapabilitiesAVCodecs() {
  InitializeAVCodecs();
}

Json::Value* DeviceCapabilitiesAVCodecs::Get() {
  Json::Value* obj = new Json::Value();
  (*obj)["audioCodecs"] = audioCodecs;
  (*obj)["videoCodecs"] = videoCodecs;
  return obj;
}

void DeviceCapabilitiesAVCodecs::InitializeAVCodecs() {
  std::set<std::string> supportedAudioCodecs;
  VideoCodecMap supportedVideoCodecs;
  GetSupportedAVCodecs(&supportedAudioCodecs, &supportedVideoCodecs);

  for (std::set<std::string>::const_iterator it = supportedAudioCodecs.begin();
       it != supportedAudioCodecs.end(); ++it) {
    Json::Value audioCodec;
    audioCodec["format"] = Json::Value(*it);
    audioCodecs.append(audioCodec);
  }

  for (VideoCodecMap::const_iterator it = supportedVideoCodecs.begin();
       it != supportedVideoCodecs.end(); ++it) {
    VideoCodec codec = it->second;
    Json::Value videoCodec;
    videoCodec["format"] = Json::Value(codec.format);
    videoCodec["hwAccel"] = Json::Value(codec.hwAccel);
    videoCodec["encode"] = Json::Value(codec.encode);
    videoCodecs.append(videoCodec);
  }
}

void DeviceCapabilitiesAVCodecs::GetAllCodecsAndFormatsFromFFmpeg(
    std::vector<std::string>* audio_codecs,
    std::vector<VideoCodec>* video_codecs,
    std::vector<std::string>* formats) {
  base::FilePath media_path;
  PathService::Get(content::DIR_MEDIA_LIBS, &media_path);
  media::InitializeMediaLibrary(media_path);
  media::FFmpegGlue::InitializeFFmpeg();

  AVCodec* codec = NULL;
  while ((codec = av_codec_next(codec))) {
    if (codec->type == AVMEDIA_TYPE_AUDIO)
      audio_codecs->push_back(std::string(codec->name));
    else if (codec->type == AVMEDIA_TYPE_VIDEO) {
      VideoCodec v_codec;
      v_codec.encode = false;
      v_codec.format = std::string(codec->name);
      // FIXME(qjia7): find how to get hwAccel value
      v_codec.hwAccel = false;
      if (av_codec_is_encoder(codec) != 0)
        v_codec.encode = true;
      video_codecs->push_back(v_codec);
    }
  }

  AVInputFormat* iformat = NULL;
  while ((iformat = av_iformat_next(iformat))) {
    std::vector<std::string> formats_out;
    base::SplitString(iformat->name, ',', &formats_out);
    for (std::vector<std::string>::iterator it = formats_out.begin();
         it != formats_out.end();
         ++it) {
      formats->push_back(*it);
    }
  }
}

void DeviceCapabilitiesAVCodecs::GetSupportedAudioCodecForMimeType(
    const std::vector<std::string>& codecs,
    std::set<std::string>* codecs_out,
    const std::string& mimetype) {
  if (!net::IsSupportedMediaMimeType(mimetype))
    return;
  for (std::vector<std::string>::const_iterator it = codecs.begin();
       it != codecs.end(); ++it) {
    std::string it_codec = *it;
    if (net::IsStrictMediaMimeType(mimetype)) {
      std::vector<std::string> strict_codecs;
      net::ParseCodecString(it_codec, &strict_codecs, false);
      if (net::IsSupportedStrictMediaMimeType(mimetype,
                                              strict_codecs)) {
        codecs_out->insert(it_codec);
      }
    } else {
      codecs_out->insert(it_codec);
    }
  }
}

void DeviceCapabilitiesAVCodecs::GetSupportedVideoCodecForMimeType(
    const std::vector<VideoCodec>& codecs,
    VideoCodecMap* codecs_out,
    const std::string& mimetype) {
  if (!net::IsSupportedMediaMimeType(mimetype))
    return;
  for (std::vector<VideoCodec>::const_iterator it = codecs.begin();
       it != codecs.end(); ++it) {
    VideoCodec it_codec = *it;
    if (net::IsStrictMediaMimeType(mimetype)) {
      std::vector<std::string> strict_codecs;
      net::ParseCodecString(it_codec.format, &strict_codecs, false);
      if (net::IsSupportedStrictMediaMimeType(mimetype,
                                              strict_codecs)) {
        (*codecs_out)[it_codec.format] = it_codec;
      }
    } else {
      (*codecs_out)[it_codec.format] = it_codec;
    }
  }
}

void DeviceCapabilitiesAVCodecs::GetSupportedAVCodecs(
    std::set<std::string>* supportedAudioCodecs,
    VideoCodecMap* supportedVideoCodecs) {
  std::vector<std::string> audioCodecsSet;
  std::vector<VideoCodec> videoCodecsSet;
  std::vector<std::string> formats;
  GetAllCodecsAndFormatsFromFFmpeg(&audioCodecsSet, &videoCodecsSet, &formats);

  for (std::vector<std::string>::const_iterator it_format = formats.begin();
       it_format != formats.end(); ++it_format) {
    std::string audio_mime_type = std::string("audio/" + *it_format);
    std::string video_mime_type = std::string("video/" + *it_format);
    GetSupportedAudioCodecForMimeType(audioCodecsSet,
                                      supportedAudioCodecs,
                                      audio_mime_type);
    GetSupportedVideoCodecForMimeType(videoCodecsSet,
                                      supportedVideoCodecs,
                                      video_mime_type);
  }
}

}  // namespace sysapps
}  // namespace xwalk
