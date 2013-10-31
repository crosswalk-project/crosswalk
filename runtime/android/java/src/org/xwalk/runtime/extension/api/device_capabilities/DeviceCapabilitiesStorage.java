// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.device_capabilities;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Environment;
import android.os.StatFs;
import android.util.Log;
import android.util.SparseArray;

import java.io.File;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.runtime.extension.XWalkExtensionContext;

public class DeviceCapabilitiesStorage {
    private static final String TAG = "DeviceCapabilitiesStorage";

    private DeviceCapabilities mDeviceCapabilities;
    private XWalkExtensionContext mExtensionContext;

    private static int mStorageCount = 0;
    // Holds all available storages.
    private final SparseArray<StorageUnit> mStorageList = new SparseArray<StorageUnit>();

    private boolean mIsListening = false;
    private IntentFilter mIntentFilter = new IntentFilter();

    class StorageUnit {
        private int mId;
        private String mName;
        private String mType;
        private long mCapacity;
        private long mAvailCapacity;
        private String mPath;

        public StorageUnit(int id, String name, String type) {
            mId = id;
            mName = name;
            mType = type;
            mPath = "";
            mCapacity = 0;
            mAvailCapacity = 0;
        }

        public int getId() { return mId; }
        public String getName() { return mName; }
        public String getType() { return mType; }
        public String getPath() { return mPath; }
        public long getCapacity() { return mCapacity; }
        public long getAvailCapacity() { return mAvailCapacity; }

        public void setType(String type) { mType = type;}
        public void setPath(String path) {
          mPath = path;
          updateCapacity();
        }

        public boolean isSame(StorageUnit unit) {
            return mPath == unit.getPath();
        }

        public boolean isValid() {
            if (mPath == null || mPath.isEmpty()) {
                mCapacity = 0;
                mAvailCapacity = 0;
                return false;
            }

            File file = new File(mPath);
            return file.canRead();
        }

        public void updateCapacity() {
            if (!isValid()) {
                return;
            }

            StatFs stat = new StatFs(mPath);
            // FIXME(halton): After API level 18, use getTotalBytes() and
            // getAvailableBytes() instead
            long blockSize = stat.getBlockSize();
            mCapacity = blockSize * stat.getBlockCount() / 1024;
            mAvailCapacity = blockSize * stat.getAvailableBlocks() / 1024;
        }

        public JSONObject convertToJSON() {
            JSONObject out = new JSONObject();

            try {
                out.put("id", mId + 1); // Display from 1
                out.put("name", mName);
                out.put("type", mType);
                out.put("capacity", mCapacity);
                out.put("availCapacity", mAvailCapacity);
            } catch (JSONException e) {
                return mDeviceCapabilities.setErrorMessage(e.toString());
            }

            return out;
        }
    }

    private final BroadcastReceiver mStorageListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                notifyAndSaveAttachedStorage();
            }

            if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)
                || Intent.ACTION_MEDIA_REMOVED.equals(action)
                || Intent.ACTION_MEDIA_BAD_REMOVAL.equals(action)) {
                notifyAndRemoveDetachedStorage();
            }
        }
    };

    public DeviceCapabilitiesStorage(DeviceCapabilities instance,
                                     XWalkExtensionContext context) {
        mDeviceCapabilities = instance;
        mExtensionContext = context;

        registerIntentFilter();

        // Fetch the original storage list
        initStorageList();
    }

    public JSONObject getInfo() {
        JSONObject out = new JSONObject();
        JSONArray arr = new JSONArray();
        try {
            for(int i = 0; i < mStorageList.size(); i++) {
                arr.put(mStorageList.valueAt(i).convertToJSON());
            }
            out.put("storages", arr);
        } catch (JSONException e) {
            return mDeviceCapabilities.setErrorMessage(e.toString());
        }

        return out;
    }

    private void initStorageList() {
        mStorageList.clear();
        mStorageCount = 0;

        StorageUnit unit = new StorageUnit(mStorageCount, "Internal", "fixed");
        unit.setPath(Environment.getRootDirectory().getAbsolutePath());
        mStorageList.put(mStorageCount, unit);
        ++mStorageCount;

        // Attempt to add emulated stroage first
        int sdcardNum = mStorageCount - 1; // sdcard count from 0
        unit = new StorageUnit(mStorageCount, new String("sdcard" + Integer.toString(sdcardNum)), "fixed");
        if (Environment.isExternalStorageRemovable()) {
            unit.setType("removable");
        }
        unit.setPath(Environment.getExternalStorageDirectory().getAbsolutePath());

        if (unit.isValid()) {
            mStorageList.put(mStorageCount, unit);
            ++mStorageCount;
        }

        // Then attempt to add real removable storage
        attemptAddExternalStorage();
    }

    private void registerIntentFilter() {
        mIntentFilter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
        mIntentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        mIntentFilter.addAction(Intent.ACTION_MEDIA_REMOVED);
        mIntentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        mIntentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        mIntentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        mIntentFilter.addDataScheme("file");
    }

    private boolean attemptAddExternalStorage() {
        int sdcardNum = mStorageCount - 1;
        StorageUnit unit = new StorageUnit(mStorageCount, new String("sdcard" + Integer.toString(sdcardNum)), "removable");
        unit.setPath("/storage/sdcard" + Integer.toString(sdcardNum));

        if (!unit.isValid()) {
            return false;
        }

        for(int i = 0; i < mStorageList.size(); i++) {
            if (unit.isSame(mStorageList.valueAt(i))) {
                return false;
            }
        }

        mStorageList.put(mStorageCount, unit);
        ++mStorageCount;
        return true;
    }

    public void registerListener() {
        if (mIsListening) {
            return;
        }

        mIsListening = true;
        mExtensionContext.getActivity().registerReceiver(mStorageListener, mIntentFilter);
    }

    public void unregisterListener() {
        if (!mIsListening) {
            return;
        }

        mIsListening = false;
        mExtensionContext.getActivity().unregisterReceiver(mStorageListener);
    }

    private void notifyAndSaveAttachedStorage() {
        if(!attemptAddExternalStorage()) {
            return;
        }

        StorageUnit unit = mStorageList.valueAt(mStorageList.size() - 1);
        JSONObject out = new JSONObject();
        try {
            out.put("reply", "attachStorage");
            out.put("eventName", "onattach");
            out.put("data", unit.convertToJSON());

            mDeviceCapabilities.broadcastMessage(out.toString());
        } catch (JSONException e) {
            mDeviceCapabilities.printErrorMessage(e);
        }

    }

    private void notifyAndRemoveDetachedStorage() {
        StorageUnit unit = mStorageList.valueAt(mStorageList.size() - 1);

        if(unit.getType() != "removable") {
            return;
        }

        JSONObject out = new JSONObject();
        try {
            out.put("reply", "detachStorage");
            out.put("eventName", "ondetach");
            out.put("data", unit.convertToJSON());

            mDeviceCapabilities.broadcastMessage(out.toString());
            mStorageList.remove(unit.getId());
            --mStorageCount;
        } catch (JSONException e) {
            mDeviceCapabilities.printErrorMessage(e);
        }
    }

    public void onResume() {
        // Fistly, check the lasted external storage is valid.
        // If not, remove it and send "ondetached" event.
        StorageUnit lastUnit = mStorageList.valueAt(mStorageList.size() - 1);
        if(!lastUnit.isValid()) {
            notifyAndRemoveDetachedStorage();
        }

        // Secondly, attmpt to add a possible external storage and send "onattached" event.
        notifyAndSaveAttachedStorage();

        registerListener();
    }

    public void onPause() {
        unregisterListener();
    }

    public void onDestroy() {
    }
}
