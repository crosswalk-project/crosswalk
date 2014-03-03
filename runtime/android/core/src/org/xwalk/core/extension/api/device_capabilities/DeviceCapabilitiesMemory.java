// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.device_capabilities;

import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.content.Context;
import android.util.Log;
import android.os.Build;

import java.io.RandomAccessFile;
import java.io.IOException;

import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.core.extension.XWalkExtensionContext;

class DeviceCapabilitiesMemory {
    private static final String MEM_INFO_FILE = "/proc/meminfo";
    private static final String TAG = "DeviceCapabilitiesMemory";

    private DeviceCapabilities mDeviceCapabilities;
    private Context mContext;

    private long mAvailableCapacity = 0;
    private long mCapacity = 0;

    public DeviceCapabilitiesMemory(DeviceCapabilities instance,
                                    XWalkExtensionContext context) {
        mDeviceCapabilities = instance;
        mContext = context.getContext();
    }

    public JSONObject getInfo() {
        readMemoryInfo();

        JSONObject out = new JSONObject();
        try {
            out.put("capacity", mCapacity);
            out.put("availCapacity", mAvailableCapacity);
        } catch (JSONException e) {
            return mDeviceCapabilities.setErrorMessage(e.toString());
        }

        return out;
    }

    private void readMemoryInfo() {
        MemoryInfo mem_info = new MemoryInfo();
        ActivityManager activityManager = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        activityManager.getMemoryInfo(mem_info);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            mCapacity = mem_info.totalMem;
        } else {
            mCapacity = getTotalMemFromFile();
        }
        mAvailableCapacity = mem_info.availMem;
    }

    private long getTotalMemFromFile() {
        long capacity = 0;
        RandomAccessFile file = null;

        try {
            file = new RandomAccessFile(MEM_INFO_FILE, "r");
            String line = file.readLine();

            String[] arrs = line.split(":");
            if (!arrs[0].equals("MemTotal")) {
                return 0;
            }
            String[] values = arrs[1].trim().split("\\s+");
            capacity = Long.parseLong(values[0]) * 1024;
        } catch (IOException e) {
            capacity = 0;
            Log.e(TAG, e.toString());
        } finally {
            try {
                if (file != null) {
                    file.close();
                }
            } catch (IOException e) {
                Log.e(TAG, e.toString());
            }
        }

        return capacity; 
    }
}
