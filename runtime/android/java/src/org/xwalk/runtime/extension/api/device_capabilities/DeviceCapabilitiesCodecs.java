// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.device_capabilities;

import org.json.JSONObject;
import org.xwalk.runtime.extension.XWalkExtensionContext;

class DeviceCapabilitiesCodecs {
    private static XWalkMediaCodec sMediaCodec;

    public DeviceCapabilitiesCodecs(DeviceCapabilities instance,
                                    XWalkExtensionContext context) {
        sMediaCodec = XWalkMediaCodec.getInstance(instance);
    }

    public JSONObject getInfo() {
        return sMediaCodec.getCodecsInfo();
    }
}
