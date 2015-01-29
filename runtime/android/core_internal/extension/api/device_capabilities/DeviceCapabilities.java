// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.device_capabilities;

import android.app.Activity;
import android.content.Context;
import android.provider.ContactsContract;
import android.util.Log;

import org.xwalk.core.internal.extension.XWalkExtensionWithActivityStateListener;

import org.chromium.base.ActivityState;
import org.json.JSONException;
import org.json.JSONObject;

public class DeviceCapabilities extends XWalkExtensionWithActivityStateListener {
    public static final String JS_API_PATH = "jsapi/device_capabilities_api.js";

    private static final String TAG = "DeviceCapabilities";
    private static final String NAME = "xwalk.experimental.system";

    private DeviceCapabilitiesCPU mCPU;
    private DeviceCapabilitiesCodecs mCodecs;
    private DeviceCapabilitiesDisplay mDisplay;
    private DeviceCapabilitiesMemory mMemory;
    private DeviceCapabilitiesStorage mStorage;

    public DeviceCapabilities(String jsApiContent, Activity activity) {
        super(NAME, jsApiContent, activity);

        Context context = activity.getApplicationContext();

        mCPU = new DeviceCapabilitiesCPU(this);
        mCodecs = new DeviceCapabilitiesCodecs(this);
        mDisplay = new DeviceCapabilitiesDisplay(this, context);
        mMemory = new DeviceCapabilitiesMemory(this, context);
        mStorage = new DeviceCapabilitiesStorage(this, activity);
    }

    private void handleMessage(int instanceID, String message) {
        try {
            JSONObject jsonInput = new JSONObject(message);
            String cmd = jsonInput.getString("cmd");

            if (cmd.equals("addEventListener")) {
                String eventName = jsonInput.getString("eventName");
                handleAddEventListener(eventName);
            } else {
                String asyncCallId = jsonInput.getString("asyncCallId");
                handleGetDeviceInfo(instanceID, asyncCallId, cmd);
            }
        } catch (JSONException e) {
            printErrorMessage(e);
        }
    }

    private void handleGetDeviceInfo(int instanceID, String asyncCallId, String cmd) {
        try {
            JSONObject jsonOutput = new JSONObject();
            if (cmd.equals("getCPUInfo")) {
                jsonOutput.put("data", mCPU.getInfo());
            } else if (cmd.equals("getCodecsInfo")) {
                jsonOutput.put("data", mCodecs.getInfo());
            } else if (cmd.equals("getDisplayInfo")) {
                jsonOutput.put("data", mDisplay.getInfo());
            } else if (cmd.equals("getMemoryInfo")) {
                jsonOutput.put("data", mMemory.getInfo());
            } else if (cmd.equals("getStorageInfo")) {
                jsonOutput.put("data", mStorage.getInfo());
            }
            jsonOutput.put("asyncCallId", asyncCallId);
            this.postMessage(instanceID, jsonOutput.toString());
        } catch (JSONException e) {
            printErrorMessage(e);
        }
    }

    private void handleAddEventListener(String eventName) {
        if (eventName.equals("storageattach") || eventName.equals("storagedetach")) {
            mStorage.registerListener();
        }
    }

    protected void printErrorMessage(JSONException e) {
        Log.e(TAG, e.toString());
    }

    protected JSONObject setErrorMessage(String error) {
        JSONObject out = new JSONObject();
        JSONObject errorMessage = new JSONObject();
        try {
            errorMessage.put("message", error);
            out.put("error", errorMessage);
        } catch (JSONException e) {
            Log.e(TAG, e.toString());
        }
        return out;
    }

    @Override
    public void onMessage(int instanceID, String message) {
        if (!message.isEmpty()) {
            handleMessage(instanceID, message);
        }
    }

    @Override
    public void onActivityStateChange(Activity activity, int newState) {
        switch (newState) {
            case ActivityState.RESUMED:
                mDisplay.onResume();
                mStorage.onResume();
                break;
            case ActivityState.PAUSED:
                mDisplay.onPause();
                mStorage.onPause();
                break;
            case ActivityState.DESTROYED:
                mDisplay.onDestroy();
                mStorage.onDestroy();
                break;
            default:
                break;
        }
    }

    @Override
    public String onSyncMessage(int instanceID, String message) {
        return null;
    }
}
