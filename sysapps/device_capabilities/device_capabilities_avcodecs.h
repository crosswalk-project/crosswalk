// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_AVCODECS_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_AVCODECS_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "third_party/jsoncpp/source/include/json/json.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities_utils.h"

namespace xwalk {
namespace sysapps {

class DeviceCapabilitiesAVCodecs : public DeviceCapabilitiesObject {
 public:
  static DeviceCapabilitiesObject& GetDeviceInstance() {
    static DeviceCapabilitiesAVCodecs instance;
    return instance;
  }
  Json::Value* Get();
  void AddEventListener(const std::string& event_name,
      DeviceCapabilitiesInstance* instance) {}
  void RemoveEventListener(DeviceCapabilitiesInstance* instance) {}

 private:
  struct VideoCodec {
    bool encode;
    bool hwAccel;
    std::string format;
  };
  typedef std::map<std::string, VideoCodec> VideoCodecMap;

  explicit DeviceCapabilitiesAVCodecs();
  void InitializeAVCodecs();
  void GetAllCodecsAndFormatsFromFFmpeg(std::vector<std::string>* audio_codecs,
                                        std::vector<VideoCodec>* video_codecs,
                                        std::vector<std::string>* formats);
  void GetSupportedAVCodecs(
      std::set<std::string>* supportedAudioCodecs,
      VideoCodecMap* supportedVideoCodecs);
  void GetSupportedAudioCodecForMimeType(
      const std::vector<std::string>& codecs,
      std::set<std::string>* out_codecs,
      const std::string& mimetype);
  void GetSupportedVideoCodecForMimeType(
      const std::vector<VideoCodec>& codecs,
      VideoCodecMap* out_codecs,
      const std::string& mimetype);

  Json::Value audioCodecs;
  Json::Value videoCodecs;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DEVICE_CAPABILITIES_AVCODECS_H_
