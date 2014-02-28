// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.device_capabilities;

import org.json.JSONObject;

class MediaCodecNull extends XWalkMediaCodec {
    public MediaCodecNull(DeviceCapabilities instance) {
    }

    @Override
    public JSONObject getCodecsInfo() {
        return new JSONObject();
    }
}
