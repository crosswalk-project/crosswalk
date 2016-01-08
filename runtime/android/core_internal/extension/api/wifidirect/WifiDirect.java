// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.wifidirect;

import android.app.Activity;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ChannelListener;
import android.net.wifi.p2p.WifiP2pManager.ConnectionInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.net.wifi.p2p.WifiP2pManager.GroupInfoListener;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.WpsInfo;
import android.util.Log;

import org.chromium.base.ActivityState;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.xwalk.core.internal.extension.XWalkExtensionWithActivityStateListener;

public class WifiDirect extends XWalkExtensionWithActivityStateListener {
    public static final String JS_API_PATH = "jsapi/wifidirect_api.js";

    private static final String TAG = "WifiDirect";
    private static final String NAME = "xwalk.experimental.wifidirect";
    
    // Tags:
    private static final String TAG_ASYNC_CALL_ID = "asyncCallId";
    private static final String TAG_CMD = "cmd";
    private static final String TAG_CONNECTED = "connected";
    private static final String TAG_DATA = "data";
    private static final String TAG_ENABLED = "enabled";
    private static final String TAG_ERROR = "error";
    private static final String TAG_ERROR_CODE = "errorCode";
    private static final String TAG_EVENT_NAME = "eventName";
    private static final String TAG_FALSE = "false";
    private static final String TAG_GROUP_FORMED = "groupFormed";
    private static final String TAG_IS_SERVER = "isServer";
    private static final String TAG_MAC = "MAC";
    private static final String TAG_MESSAGE = "message";
    private static final String TAG_NAME = "name";
    private static final String TAG_SERVER_IP = "serverIP";
    private static final String TAG_STATUS = "status";
    private static final String TAG_TRUE = "true";
    private static final String TAG_TYPE = "type";
    
    // Commands:
    private static final String CMD_CANCEL_CONNECT = "cancelConnect";
    private static final String CMD_CONNECT = "connect";
    private static final String CMD_DISCONNECT = "disconnect";
    private static final String CMD_DISCOVER_PEERS = "discoverPeers";
    private static final String CMD_GET_CONNECTION_INFO = "getConnectionInfo";
    private static final String CMD_GET_PEERS = "getPeers";
    private static final String CMD_INIT = "init";
    
    // Events:
    private static final String EVENT_CONNECTION_CHANGED = "connectionchanged";
    private static final String EVENT_DISCOVERY_STOPPED = "discoverystoppedevent";
    private static final String EVENT_PEERS_CHANGED = "peerschanged";
    private static final String EVENT_THIS_DEVICE_CHANGED = "thisdevicechanged";
    private static final String EVENT_WIFI_STATE_CHANGED = "wifistatechanged";
    
    // States:
    private static final String STATE_AVAILABLE = "available";
    private static final String STATE_CONNECTED = "connected";
    private static final String STATE_FAILED = "failed";
    private static final String STATE_INVITED = "invited";
    private static final String STATE_UNAVAILABLE = "unavailable";
    
    // Errors:
    private static final String ERROR_P2P_UNSUPPORTED = "WifiP2pManager.P2P_UNSUPPORTED";
    private static final String ERROR_NO_SERVICE_REQUESTS = "WifiP2pManager.NO_SERVICE_REQUESTS";
    private static final String ERROR_BUSY = "WifiP2pManager.BUSY";
    private static final String ERROR_DEFAULT = "WifiP2pManager.ERROR";
    
    // Error messages:
    private static final String ERROR_INVALID_CALL_NO_DATA_MSG = "Error: Invalid connect API call - data === null";
    private static final String ERROR_GENERAL_ERROR_MSG_STEM = "Android WiFi Direct error: ";
    private static final String ERROR_REASON_CODE_STEM = "WifiP2pManager reasonCode: ";

    private WifiP2pManager mManager = null;
    private Channel mChannel = null;
    private BroadcastReceiver mReceiver = null;
    private IntentFilter mIntentFilter;
    private boolean mReceiverRegistered = false;
    private Activity mActivity = null;
    
    public WifiDirect(String jsApiContent, Activity activity) {
        super(NAME, jsApiContent, activity);
        mActivity = activity;
    }

    private WifiP2pManager.ActionListener createCallActionListener(final int instanceID, final JSONObject jsonOutput) {
        return new WifiP2pManager.ActionListener() {
            @Override
            public void onSuccess() {
                try {
                    jsonOutput.put(TAG_DATA, true);
                } catch (JSONException e) {
                    printErrorMessage(e);
                }
                WifiDirect.this.postMessage(instanceID, jsonOutput.toString());
            }

            @Override
            public void onFailure(int reasonCode) {
                setError(jsonOutput, "", reasonCode);
                WifiDirect.this.postMessage(instanceID, jsonOutput.toString());
            }
        };
    }
    
    private void disconnect(final int instanceID, final JSONObject jsonOutput) {
        if (mManager != null && mChannel != null) {
            mManager.requestGroupInfo(mChannel, new GroupInfoListener() {
                @Override
                public void onGroupInfoAvailable(WifiP2pGroup group) {
                    if (group != null) {
                        mManager.removeGroup(mChannel, createCallActionListener(instanceID, jsonOutput));
                    }
                }
            });
        }
    }
    
    private void handleMessage(final int instanceID, String message) {
        try {
            JSONObject jsonInput = new JSONObject(message);
            String cmd = jsonInput.getString(TAG_CMD);
            final String asyncCallId = jsonInput.getString(TAG_ASYNC_CALL_ID);
        
            final JSONObject jsonOutput = new JSONObject();
            jsonOutput.put(TAG_ASYNC_CALL_ID, asyncCallId);
            if (cmd.equals(CMD_DISCOVER_PEERS)) {
                mManager.discoverPeers(mChannel, createCallActionListener(instanceID, jsonOutput));
            } else if (cmd.equals(CMD_GET_PEERS)) {
                mManager.requestPeers(mChannel, new WifiP2pManager.PeerListListener() {
                    @Override
                    public void onPeersAvailable(WifiP2pDeviceList peers) {
                        try {
                            jsonOutput.put(TAG_DATA, convertListToJSON(peers));
                        } catch (JSONException e) {
                            printErrorMessage(e);
                        }
                        WifiDirect.this.postMessage(instanceID, jsonOutput.toString());
                    }
                });
            } else if (cmd.equals(CMD_INIT)) {
                jsonOutput.put(TAG_DATA, init());
                this.postMessage(instanceID, jsonOutput.toString());
            } else if (cmd.equals(CMD_GET_CONNECTION_INFO)) {
                mManager.requestConnectionInfo(mChannel, new ConnectionInfoListener() {
                    @Override
                    public void onConnectionInfoAvailable(WifiP2pInfo info) { 
                        try {
                            JSONObject data = new JSONObject();
                            jsonOutput.put(TAG_DATA, data);
                            data.put(TAG_GROUP_FORMED, info.groupFormed);
                            if (info.groupFormed) {
                                data.put(TAG_IS_SERVER, info.isGroupOwner);
                                data.put(TAG_SERVER_IP, info.isGroupOwner ? "" : info.groupOwnerAddress.toString().replace("/", ""));
                            }
                            WifiDirect.this.postMessage(instanceID, jsonOutput.toString());
                        } catch (JSONException e) {
                            printErrorMessage(e);
                        }
                    }
                });
            } else if (cmd.equals(CMD_CONNECT)) {
                JSONObject dev = jsonInput.getJSONObject(TAG_DATA);
                if (dev == null) {
                    setError(jsonOutput, ERROR_INVALID_CALL_NO_DATA_MSG, 0);
                    WifiDirect.this.postMessage(instanceID, jsonOutput.toString());
                } else {
                    WifiP2pConfig config = new WifiP2pConfig();
                    config.deviceAddress = dev.getString(TAG_MAC);
                    config.wps.setup = WpsInfo.PBC;
                    mManager.connect(mChannel, config, createCallActionListener(instanceID, jsonOutput));
                }
            } else if (cmd.equals(CMD_CANCEL_CONNECT)) {
                mManager.cancelConnect(mChannel, createCallActionListener(instanceID, jsonOutput));
            } else if (cmd.equals(CMD_DISCONNECT)) {
                disconnect(instanceID, jsonOutput);
            }
        } catch (JSONException e) {
            printErrorMessage(e);
        }
    }
    
    private JSONObject setEventData(JSONObject out, String eventName) throws JSONException {
        out.put(TAG_EVENT_NAME, eventName);
        JSONObject data = new JSONObject();
        out.put(TAG_DATA, data);
        return data;
    }
    
    private boolean init() {
        if (mActivity == null)
            return false;
        mManager = (WifiP2pManager) mActivity.getSystemService(Context.WIFI_P2P_SERVICE);
        mChannel =  (Channel) mManager.initialize(mActivity, mActivity.getMainLooper(), null); 
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                JSONObject out = new JSONObject();
                try {
                    if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
                        setEventData(out, EVENT_WIFI_STATE_CHANGED).put(TAG_ENABLED,
                                (intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, -1) == WifiP2pManager.WIFI_P2P_STATE_ENABLED) ?
                                        TAG_TRUE : TAG_FALSE);
                    } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
                        setEventData(out, EVENT_PEERS_CHANGED);
                    } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
                        NetworkInfo networkInfo = (NetworkInfo) intent
                                .getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
                        setEventData(out, EVENT_CONNECTION_CHANGED).put(TAG_CONNECTED, networkInfo.isConnected());
                    } else if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
                        // Respond to this device's wifi state changing
                        WifiP2pDevice dev = (WifiP2pDevice)intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_DEVICE);
                        convertDeviceToJSON(setEventData(out, EVENT_THIS_DEVICE_CHANGED), dev);
                    } else if (WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
                        if (intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE, -1) != WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED)
                            return;
                        setEventData(out, EVENT_DISCOVERY_STOPPED);
                    }
                    WifiDirect.this.broadcastMessage(out.toString());
                } catch (JSONException e) {
                    WifiDirect.this.printErrorMessage(e);
                }
            }
        };
        //To define the filter in the BroadcastReceiver
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
        mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
        if (!mReceiverRegistered) {
            mActivity.registerReceiver(mReceiver, mIntentFilter);
            mReceiverRegistered = true;
        }
        return true;
    }
    
    private String convertStateToString(int state) {
        switch (state) {
        case WifiP2pDevice.AVAILABLE:
            return STATE_AVAILABLE;
        case WifiP2pDevice.CONNECTED:
            return STATE_CONNECTED;
        case WifiP2pDevice.FAILED:
            return STATE_FAILED;
        case WifiP2pDevice.INVITED:
            return STATE_INVITED;
        case WifiP2pDevice.UNAVAILABLE:
            return STATE_UNAVAILABLE;
        default:
            return "";
        }
    }
    
    private void convertDeviceToJSON(JSONObject ob, WifiP2pDevice peer) throws JSONException {
        ob.put(TAG_MAC, peer.deviceAddress);
        ob.put(TAG_NAME, peer.deviceName);
        ob.put(TAG_TYPE, peer.primaryDeviceType);
        ob.put(TAG_STATUS, convertStateToString(peer.status));
    }
    
    private JSONArray convertListToJSON(WifiP2pDeviceList peers) throws JSONException {
        JSONArray arr = new JSONArray();
        for (WifiP2pDevice peer : peers.getDeviceList()) {
            JSONObject ob = new JSONObject();
            convertDeviceToJSON(ob, peer);
            arr.put(ob);
        }
        return arr;
    }

    protected void printErrorMessage(JSONException e) {
        Log.e(TAG, e.toString());
    }

    private String convertReasonCodeToString(int reasonCode) {
        switch(reasonCode) {
        case WifiP2pManager.BUSY:
            return ERROR_BUSY;
        case WifiP2pManager.ERROR:
            return ERROR_DEFAULT;
        case WifiP2pManager.NO_SERVICE_REQUESTS:
            return ERROR_NO_SERVICE_REQUESTS;
        case WifiP2pManager.P2P_UNSUPPORTED:
            return ERROR_P2P_UNSUPPORTED;
        default:
            return ERROR_REASON_CODE_STEM + reasonCode;
        }
    }
    
    protected void setError(JSONObject out, String errorMessage, int reasonCode) {
        JSONObject data = new JSONObject();
        JSONObject error = new JSONObject();
        try {
            out.put(TAG_DATA, data);
            error.put(TAG_MESSAGE, errorMessage.isEmpty() ? ERROR_GENERAL_ERROR_MSG_STEM + convertReasonCodeToString(reasonCode) : errorMessage);
            error.put(TAG_ERROR_CODE, reasonCode);
            data.put(TAG_ERROR, error);
        } catch (JSONException e) {
            Log.e(TAG, e.toString());
        }
    }

    @Override
    public void onMessage(int instanceID, String message) {
        if (!message.isEmpty()) {
            handleMessage(instanceID, message);
        }
    }

    @Override
    public void onActivityStateChange(Activity activity, int newState) {
        if (mReceiver == null) {
            return;
        }
        switch (newState) {
            case ActivityState.RESUMED:
                mActivity = activity;
                if (!mReceiverRegistered) {
                    mActivity.registerReceiver(mReceiver, mIntentFilter);
                    mReceiverRegistered = true;
                }
                break;
            case ActivityState.PAUSED:
                if (mReceiverRegistered) {
                    mActivity.unregisterReceiver(mReceiver);
                    mReceiverRegistered = false;
                }
                mActivity = null;
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
