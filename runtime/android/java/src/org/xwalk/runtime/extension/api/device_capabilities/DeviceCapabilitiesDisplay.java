// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.device_capabilities;

import android.content.Context;
import android.graphics.Point;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.SparseArray;
import android.view.Display;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.runtime.extension.api.XWalkDisplayManager;
import org.xwalk.runtime.extension.XWalkExtensionContext;

public class DeviceCapabilitiesDisplay {
    private static final String TAG = "DeviceCapabilitiesDisplay";

    private DeviceCapabilities mDeviceCapabilities;
    private XWalkDisplayManager mDisplayManager;

    // Holds all available displays connected to the system.
    private final SparseArray<Display> mDisplayList = new SparseArray<Display>();

    private final XWalkDisplayManager.DisplayListener mDisplayListener =
            new XWalkDisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int displayId) {
            // Broadcast and add the added display to JavaScript
            notifyAndSaveConnectedDisplay(mDisplayManager.getDisplay(displayId));
        }

        @Override
        public void onDisplayRemoved(int displayId) {
            Display disp = mDisplayList.get(displayId);

            // Do nothing if the display does not exsit on cache.
            if (disp == null) {
                return;
            }

            // Broadcast and remove the added display to JavaScript
            notifyAndRemoveDisconnectedDisplay(disp);
        }

        @Override
        public void onDisplayChanged(int displayId) {
        }
    };

    public DeviceCapabilitiesDisplay(DeviceCapabilities instance,
                                     XWalkExtensionContext context) {
        mDeviceCapabilities = instance;
        mDisplayManager = XWalkDisplayManager.getInstance(context.getContext());

        // Fetch the original display list
        initDisplayList();
    }

    public JSONObject getInfo() {
        JSONObject out = new JSONObject();
        JSONArray arr = new JSONArray();

        try {
            for(int i = 0; i < mDisplayList.size(); i++) {
                arr.put(convertDisplayToJSON(mDisplayList.valueAt(i)));
            }
            out.put("displays", arr);
        } catch (JSONException e) {
            return mDeviceCapabilities.setErrorMessage(e.toString());
        }

        return out;
    }

    public JSONObject convertDisplayToJSON(Display disp) {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        disp.getRealMetrics(displayMetrics);

        Point realSize = new Point();
        disp.getRealSize(realSize);

        Point availSize = new Point();
        disp.getSize(availSize);

        JSONObject out = new JSONObject();
        try {
            out.put("id", disp.getDisplayId());
            out.put("name", disp.getName());
            out.put("isPrimary", disp.getDisplayId() == disp.DEFAULT_DISPLAY);
            out.put("isInternal", disp.getDisplayId() == disp.DEFAULT_DISPLAY);
            out.put("dpiX", (int) displayMetrics.xdpi);
            out.put("dpiY", (int) displayMetrics.ydpi);
            out.put("width", realSize.x);
            out.put("height", realSize.y);
            out.put("availWidth", availSize.x);
            out.put("availHeight", availSize.y);
        } catch (JSONException e) {
            return mDeviceCapabilities.setErrorMessage(e.toString());
        }
        return out;
    }

    private void initDisplayList() {
        Display[] displays = mDisplayManager.getDisplays();

        for (Display disp : displays) {
            mDisplayList.put(disp.getDisplayId(), disp);
        }
    }

    private void notifyAndSaveConnectedDisplay(Display disp) {
        if (disp == null) {
            return;
        }

        JSONObject out = new JSONObject();
        try {
            out.put("reply", "connectDisplay");
            out.put("eventName", "onconnect");
            out.put("data", convertDisplayToJSON(disp));

            mDeviceCapabilities.broadcastMessage(out.toString());
            mDisplayList.put(disp.getDisplayId(), disp);
        } catch (JSONException e) {
            mDeviceCapabilities.printErrorMessage(e);
        }
    }

    private void notifyAndRemoveDisconnectedDisplay(Display disp) {
        JSONObject out = new JSONObject();
        try {
            out.put("reply", "disconnectDisplay");
            out.put("eventName", "ondisconnect");
            out.put("data", convertDisplayToJSON(disp));

            mDeviceCapabilities.broadcastMessage(out.toString());
            mDisplayList.remove(disp.getDisplayId());
        } catch (JSONException e) {
            mDeviceCapabilities.printErrorMessage(e);
        }
    }

    public void onResume() {
        Display[] displays = mDisplayManager.getDisplays();

        // Firstly, check whether display in latest list is in cached display list.
        // If not found, then send out "onconnect" message and insert to cache.
        // If found, only update the display object without sending message.
        for (Display disp : displays) {
            Display foundDisplay = mDisplayList.get(disp.getDisplayId());
            if (foundDisplay == null) {
                notifyAndSaveConnectedDisplay(disp);
            } else {
                mDisplayList.put(disp.getDisplayId(), disp);
            }
        }

        // Secondly, remove those displays that only in cache.
        for(int i = 0; i < mDisplayList.size(); i++) {
            boolean found = false;
            for (Display disp : displays) {
                if (mDisplayList.valueAt(i).getDisplayId() == disp.getDisplayId()) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                notifyAndRemoveDisconnectedDisplay(mDisplayList.valueAt(i));
            }
        }

        // Register the listener to display manager.
        //
        // XWalkDisplayManager.registerDisplayListener only works on UI thread,
        // otherwise coredump.
        //
        // Register the listener here lead to a connect/disconnect event will
        // unconditionally be posted to JS whatever there is a JS listener
        // registered or not. It is kind of waste of resource.
        //
        // Fortunately, the listneres map in JSAPI will ensurce no noise messages
        // will exposed out.
        mDisplayManager.registerDisplayListener(mDisplayListener);
    }

    public void onPause() {
        mDisplayManager.unregisterDisplayListener(mDisplayListener);
    }

    public void onDestroy() {
    }
}
