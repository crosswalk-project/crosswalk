// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.device_capabilities;

import org.json.JSONObject;
import org.xwalk.core.internal.extension.XWalkExtensionContext;

class DeviceCapabilitiesCodecs {
    private XWalkMediaCodec mediaCodec;

    public DeviceCapabilitiesCodecs(DeviceCapabilities instance,
                                    XWalkExtensionContext context) {
        mediaCodec = XWalkMediaCodec.Create(instance);
    }

    public JSONObject getInfo() {
        return mediaCodec.getCodecsInfo();
    }
}
