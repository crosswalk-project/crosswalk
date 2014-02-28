// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.device_capabilities;

import java.util.HashSet;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

class MediaCodec extends XWalkMediaCodec {
    public MediaCodec(DeviceCapabilities instance) {
        mDeviceCapabilities = instance;

        mAudioCodecsSet = new HashSet<AudioCodecElement>();
        mVideoCodecsSet = new HashSet<VideoCodecElement>();

        // Suppose the codecs that MediaCodec reports won't change during runtime,
        // so initialize in constructor.
        getCodecsList();
    }

    @Override
    public JSONObject getCodecsInfo() {
        JSONObject outputObject = new JSONObject();
        JSONArray audioCodecsArray = new JSONArray();
        JSONArray videoCodecsArray = new JSONArray();

        try {
            for (AudioCodecElement codecToAdd : mAudioCodecsSet) {
                JSONObject codecsObject = new JSONObject();
                codecsObject.put("format", codecToAdd.codecName);
                audioCodecsArray.put(codecsObject);
            }
            for (VideoCodecElement codecToAdd : mVideoCodecsSet) {
                JSONObject codecsObject = new JSONObject();
                codecsObject.put("format", codecToAdd.codecName);
                codecsObject.put("encode", codecToAdd.isEncoder);
                codecsObject.put("hwAccel", codecToAdd.hwAccel);
                videoCodecsArray.put(codecsObject);
            }

            outputObject.put("audioCodecs", audioCodecsArray);
            outputObject.put("videoCodecs", videoCodecsArray);
        } catch (JSONException e) {
            return mDeviceCapabilities.setErrorMessage(e.toString());
        }

        return outputObject;
    }

    public void getCodecsList() {
        int numCodecs = android.media.MediaCodecList.getCodecCount();
        for (int i = 0; i < numCodecs; i++) {
            android.media.MediaCodecInfo codecInfo =
                    android.media.MediaCodecList.getCodecInfoAt(i);
            String name = codecInfo.getName().toUpperCase();
            boolean hasAdded = false;

            for (String nameListElement : AUDIO_CODEC_NAMES) {
                if (name.contains(nameListElement)) {
                    mAudioCodecsSet.add(new AudioCodecElement(nameListElement));
                    hasAdded = true;
                    break;
                }
            }

            if (hasAdded) {
                continue;
            }

            for (String nameListElement : VIDEO_CODEC_NAMES) {
                if (name.contains(nameListElement)) {
                    boolean isEncoder = codecInfo.isEncoder();
                    // FIXME(fyraimar): Get the right hwAccel value.
                    mVideoCodecsSet.add(new VideoCodecElement(
                            nameListElement, isEncoder, false));
                    break;
                }
            }
        }
    }
}
