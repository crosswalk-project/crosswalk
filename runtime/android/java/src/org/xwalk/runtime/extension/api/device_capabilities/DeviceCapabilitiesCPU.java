// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.device_capabilities;

import android.util.Log;

import java.io.RandomAccessFile;
import java.io.IOException;

import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.runtime.extension.XWalkExtensionContext;

public class DeviceCapabilitiesCPU {
    private static final String SYSTEM_INFO_STAT_FILE = "/proc/stat";
    private static final String TAG = "DeviceCapabilitiesCPU";

    private DeviceCapabilities mDeviceCapabilities;

    private int mCoreNum = 0;
    private String mCPUArch = "Unknown";
    private double mCPULoad = 0.0;

    public DeviceCapabilitiesCPU(DeviceCapabilities instance,
                                 XWalkExtensionContext context) {
        mDeviceCapabilities = instance;

        // Get arch and core number at constructor since they won't change time to time.
        mCoreNum = Runtime.getRuntime().availableProcessors();
        mCPUArch = System.getProperty("os.arch");
    }

    public JSONObject getInfo() {
        getCPULoad();

        JSONObject out = new JSONObject();
        try {
            out.put("numOfProcessors", mCoreNum);
            out.put("archName", mCPUArch);
            out.put("load", mCPULoad);
        } catch (JSONException e) {
            return mDeviceCapabilities.setErrorMessage(e.toString());
        }

        return out;
    }

    /**
     * The algorithm here can be found at:
     * http://stackoverflow.com/questions/3017162/how-to-get-total-cpu-usage-in-linux-c
     */
    private boolean getCPULoad() {
        try {
            RandomAccessFile file = new RandomAccessFile(SYSTEM_INFO_STAT_FILE, "r");
            String line = file.readLine();

            String[] arrs = line.split(" ");
            long used1 = Long.parseLong(arrs[2]) + Long.parseLong(arrs[3])
                         + Long.parseLong(arrs[4]);
            long total1 = used1
                          + Long.parseLong(arrs[5]) + Long.parseLong(arrs[6])
                          + Long.parseLong(arrs[7]) + Long.parseLong(arrs[8]);
            try {
                Thread.sleep(360);
            } catch (Exception e) {
                mCPULoad = 0.0;
                return false;
            }

            file.seek(0);
            line = file.readLine();
            file.close();

            arrs = line.split(" ");
            long used2 = Long.parseLong(arrs[2]) + Long.parseLong(arrs[3])
                         + Long.parseLong(arrs[4]);
            long total2 = used1
                          + Long.parseLong(arrs[5]) + Long.parseLong(arrs[6])
                          + Long.parseLong(arrs[7]) + Long.parseLong(arrs[8]);

            if (total2 == total1) {
                mCPULoad = 0.0;
            } else {
                mCPULoad = (double) (used2 - used1) / (total2 - total1);
            }
        } catch (IOException e) {
            mCPULoad = 0.0;
            return false;
        }
        return true;
    }
}
