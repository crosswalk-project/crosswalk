// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.device_capabilities;

import android.util.Log;

import java.io.RandomAccessFile;
import java.io.IOException;

import org.json.JSONException;
import org.json.JSONObject;

class DeviceCapabilitiesCPU {
    private static final String SYSTEM_INFO_STAT_FILE = "/proc/stat";
    private static final String TAG = "DeviceCapabilitiesCPU";

    private DeviceCapabilities mDeviceCapabilities;

    private int mCoreNum = 0;
    private String mCPUArch = "Unknown";
    private double mCPULoad = 0.0;

    public DeviceCapabilitiesCPU(DeviceCapabilities instance) {
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

            String[] arrs = line.split("\\s+");
            long total1 = 0;
            for (int i = 1; i < arrs.length; ++i) {
                total1 += Long.parseLong(arrs[i]);
            }
            // arrs[4] is the time spent in idle tasks.
            long used1 = total1 - Long.parseLong(arrs[4]);
            try {
                Thread.sleep(1000);
            } catch (Exception e) {
                mCPULoad = 0.0;
                return false;
            }

            file.seek(0);
            line = file.readLine();
            file.close();

            arrs = line.split("\\s+");
            long total2 = 0;
            for (int i = 1; i < arrs.length; ++i) {
                total2 += Long.parseLong(arrs[i]);
            }
            // arrs[4] is the time spent in idle tasks.
            long used2 = total2 - Long.parseLong(arrs[4]);
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
