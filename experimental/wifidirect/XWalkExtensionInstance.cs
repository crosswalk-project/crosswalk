// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using Windows.Devices.Enumeration;
using Windows.Devices.WiFiDirect;
using Windows.Networking.Sockets;

namespace xwalk
{
    using JSONObject = Dictionary<String, Object>;

    public class XWalkExtensionInstance {
        // Tags:
        const String TAG_CMD = "cmd";
        const String TAG_CONNECTED = "connected";
        const String TAG_DATA = "data";
        const String TAG_ENABLED = "enabled";
        const String TAG_ERROR = "error";
        const String TAG_ERROR_CODE = "errorCode";
        const String TAG_EVENT_NAME = "eventName";
        const String TAG_FALSE = "false";
        const String TAG_GROUP_FORMED = "groupFormed";
        const String TAG_IS_SERVER = "isServer";
        const String TAG_MAC = "MAC";
        const String TAG_MESSAGE = "message";
        const String TAG_NAME = "name";
        const String TAG_SERVER_IP = "serverIP";
        const String TAG_CLIENT_IP = "clientIP";
        const String TAG_STATUS = "status";
        const String TAG_TRUE = "true";
        const String TAG_TYPE = "type";

        // Commands:
        const String CMD_CANCEL_CONNECT = "cancelConnect";
        const String CMD_CONNECT = "connect";
        const String CMD_DISCONNECT = "disconnect";
        const String CMD_DISCOVER_PEERS = "discoverPeers";
        const String CMD_GET_CONNECTION_INFO = "getConnectionInfo";
        const String CMD_GET_PEERS = "getPeers";
        const String CMD_INIT = "init";

        // Events:
        const String EVENT_CONNECTION_CHANGED = "connectionchanged";
        const String EVENT_DISCOVERY_STOPPED = "discoverystoppedevent";
        const String EVENT_PEERS_CHANGED = "peerschanged";
        const String EVENT_WIFI_STATE_CHANGED = "wifistatechanged";

        // States:
        const String STATE_AVAILABLE = "available";
        const String STATE_CONNECTED = "connected";
        const String STATE_UNAVAILABLE = "unavailable";

        // Error messages:
        const String ERROR_INVALID_CALL_NO_DATA_MSG = "Error: Invalid connect API call - data === null";
        const String ERROR_INVALID_CALL_DEVICE_NOT_AVAILABLE = "Error: Invalid connect API call - device not available: ";
        const String ERROR_INVALID_CALL_CMD = "Error: Invalid connect API command - cmd:";

        WiFiDirectAdvertisementPublisher mAdvertiser;
        WiFiDirectConnectionListener mConnectionListener;
        DeviceWatcher mDeviceWatcher;
        Collection<DeviceInformation> mDiscovered = new Collection<DeviceInformation>();
        Dictionary<string, WiFiDirectDevice> mConnected = new Dictionary<string, WiFiDirectDevice>();
        bool mGroupOwner = false;

        public XWalkExtensionInstance(dynamic native) {
            native_ = native;
        }

        public void HandleMessage(String message) {
            JSONObject jsonInput = new JavaScriptSerializer().Deserialize<Dictionary<String, Object>>(message);
            String cmd = jsonInput.ContainsKey(TAG_CMD) ? (String)jsonInput[TAG_CMD] : "";
            JSONObject jsonOutput = jsonInput;

            if (cmd.Equals(CMD_DISCOVER_PEERS)) {
                discoverPeers(jsonOutput);
            } else if (cmd.Equals(CMD_GET_PEERS)) {
                getPeers(jsonOutput);
            } else if (cmd.Equals(CMD_INIT)) {
                jsonOutput[TAG_DATA] = TAG_TRUE;
                native_.PostMessageToJS(new JavaScriptSerializer().Serialize(jsonOutput));
            } else if (cmd.Equals(CMD_GET_CONNECTION_INFO)) {
                getConnectionInfo(jsonOutput);
            } else if (cmd.Equals(CMD_CONNECT)) {
                connect(jsonOutput);
            } else if (cmd.Equals(CMD_CANCEL_CONNECT)) {
                removeConnected(jsonOutput);
            } else if (cmd.Equals(CMD_DISCONNECT)) {
                removeConnected(jsonOutput);
            } else {
                setError(jsonOutput, ERROR_INVALID_CALL_CMD + cmd, 0);
                native_.PostMessageToJS(new JavaScriptSerializer().Serialize(jsonOutput));
            }
        }

        public void HandleSyncMessage(String message) {
            native_.SendSyncReply(message);
        }

        DeviceInformation findDiscoveredById(string id) {
            foreach (DeviceInformation deviceInfo in mDiscovered) {
                if (deviceInfo.Id == id) {
                    return deviceInfo;
                }
            }
            return null;
        }

        // Used for lookup into mConnected as a deviceId returned from different
        // APIs could differ.
        private String extractMAC(String deviceId) {
            if (deviceId.EndsWith("_PendingRequest"))
                deviceId = deviceId.Substring(0, deviceId.Length - 15);
            if (deviceId.StartsWith("WiFiDirect#"))
                deviceId = deviceId.Substring(11);
            return deviceId.ToLower();
        }

        Dictionary<String, object> convertDeviceToJSON(DeviceInformation peer) {
            Dictionary<String, object> ob = new Dictionary<string, object>();
            ob[TAG_NAME] = peer.Name;
            ob[TAG_MAC] = peer.Id;
            String mac = extractMAC(peer.Id);
            WiFiDirectDevice device;
            mConnected.TryGetValue(mac, out device);
            if (device != null && device.ConnectionStatus == WiFiDirectConnectionStatus.Connected) {
                ob[TAG_STATUS] = STATE_CONNECTED;
            } else {
                ob[TAG_STATUS] = peer.Pairing.CanPair ? STATE_AVAILABLE : STATE_UNAVAILABLE;
            }

            if (peer.Properties != null) {
                foreach (String key in peer.Properties.Keys) {
                    var value = peer.Properties[key];
                    if (value != null)
                        ob[key] = value.ToString();
                }
            }
            return ob;
        }

        List<object> convertListToJSON(ICollection<DeviceInformation> deviceList) {
            List<object> arr = new List<object>();
            foreach (DeviceInformation peer in deviceList)
                arr.Add(convertDeviceToJSON(peer));
            return arr;
        }

        void setError(JSONObject output, String errorMessage, int reasonCode) {
            JSONObject data = new JSONObject();
            JSONObject error = new JSONObject();
            output[TAG_DATA] = data;
            error[TAG_MESSAGE] = errorMessage;
            error[TAG_ERROR_CODE] = reasonCode;
            data[TAG_ERROR] = error;
            Debug.WriteLine("setError : " + errorMessage);
        }

        JSONObject createEventData(String eventName) {
            JSONObject eventData = new JSONObject();
            eventData[TAG_EVENT_NAME] = eventName;
            eventData[TAG_DATA] = new JSONObject();
            return eventData;
        }

        void getConnectionInfo(JSONObject jsonOutput) {
            JSONObject data = new JSONObject();
            jsonOutput[TAG_DATA] = data;
            IEnumerator<WiFiDirectDevice> it = mConnected.Values.GetEnumerator();

            if (it.MoveNext()) {
                var wfdDevice = it.Current;
                var endpointPairs = wfdDevice.GetConnectionEndpointPairs();
                if (wfdDevice.ConnectionStatus == WiFiDirectConnectionStatus.Connected) {
                    if (mGroupOwner) {
                        data[TAG_GROUP_FORMED] = TAG_TRUE;
                        data[TAG_IS_SERVER] = TAG_TRUE;
                        data[TAG_SERVER_IP] = endpointPairs[0].LocalHostName;
                        data[TAG_CLIENT_IP] = endpointPairs[0].RemoteHostName;
                    } else {
                        data[TAG_GROUP_FORMED] = TAG_TRUE;
                        data[TAG_IS_SERVER] = TAG_FALSE;
                        data[TAG_SERVER_IP] = endpointPairs[0].RemoteHostName;
                        data[TAG_CLIENT_IP] = endpointPairs[0].LocalHostName;
                    }
                }
            }

            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(jsonOutput));
        }

        void removeConnected(JSONObject jsonOutput) {
            IEnumerator<WiFiDirectDevice> it = mConnected.Values.GetEnumerator();

            string result = TAG_FALSE;
            while (it.MoveNext()) {
                var wfdDevice = it.Current;
                wfdDevice.Dispose();
                result = TAG_TRUE;
            }
            jsonOutput[TAG_DATA] = result;
            mConnected.Clear();

            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(jsonOutput));
        }

        void discoverPeers(Dictionary<String, Object> jsonOutput) {
            try {
                advertise();
                mDiscovered.Clear();
                mDeviceWatcher = DeviceInformation.CreateWatcher(
                    WiFiDirectDevice.GetDeviceSelector(WiFiDirectDeviceSelectorType.AssociationEndpoint),
                    new string[] { "System.Devices.WiFiDirect.InformationElements" });

                mDeviceWatcher.Added += OnDeviceAdded;
                mDeviceWatcher.Removed += OnDeviceRemoved;
                mDeviceWatcher.Updated += OnDeviceUpdated;
                mDeviceWatcher.Stopped += OnStopped;

                mDeviceWatcher.Start();
                jsonOutput[TAG_DATA] = TAG_TRUE;
            } catch (Exception ex) {
                setError(jsonOutput, ex.ToString(), ex.HResult);
            }
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(jsonOutput));
        }

        void getPeers(JSONObject response) {
            response[TAG_DATA] = convertListToJSON(mDiscovered);
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(response));
        }

        void OnStatusChanged(WiFiDirectAdvertisementPublisher sender, WiFiDirectAdvertisementPublisherStatusChangedEventArgs statusEventArgs) {
            Debug.WriteLine("Advertisement: Status: " + statusEventArgs.Status.ToString() + " Error: " + statusEventArgs.Error.ToString());
        }

        #region DeviceWatcherEvents
        void OnDeviceAdded(DeviceWatcher deviceWatcher, DeviceInformation deviceInfo) {
            mDiscovered.Add(deviceInfo);
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(createEventData(EVENT_PEERS_CHANGED)));
        }

        void OnDeviceRemoved(DeviceWatcher deviceWatcher, DeviceInformationUpdate deviceInfoUpdate) {
            foreach (DeviceInformation deviceInfo in mDiscovered) {
                if (deviceInfo.Id == deviceInfoUpdate.Id) {
                    mDiscovered.Remove(deviceInfo);
                    break;
                }
            }
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(createEventData(EVENT_PEERS_CHANGED)));
        }

        void OnDeviceUpdated(DeviceWatcher deviceWatcher, DeviceInformationUpdate deviceInfoUpdate) {
            DeviceInformation deviceInfo = findDiscoveredById(deviceInfoUpdate.Id);
            if (deviceInfo != null) {
                deviceInfo.Update(deviceInfoUpdate);
                native_.PostMessageToJS(new JavaScriptSerializer().Serialize(createEventData(EVENT_PEERS_CHANGED)));
            }
        }

        void OnStopped(DeviceWatcher deviceWatcher, object o) {
            mDeviceWatcher.Added -= OnDeviceAdded;
            mDeviceWatcher.Removed -= OnDeviceRemoved;
            mDeviceWatcher.Updated -= OnDeviceUpdated;
            mDeviceWatcher.Stopped -= OnStopped;
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(createEventData(EVENT_DISCOVERY_STOPPED)));
        }
        #endregion DeviceWatcherEvents

        #region Inviting
        void connect(JSONObject response) {
            WiFiDirectDevice wfdDevice = null;
            try {
                JSONObject device = (JSONObject)response[TAG_DATA];
                if (device == null) {
                    setError(response, ERROR_INVALID_CALL_NO_DATA_MSG, 0);
                    native_.PostMessageToJS(new JavaScriptSerializer().Serialize(response));
                    return;
                }

                DeviceInformation deviceInfo = findDiscoveredById((string)device[TAG_MAC]);
                if (deviceInfo == null) {
                    setError(response, ERROR_INVALID_CALL_DEVICE_NOT_AVAILABLE + device[TAG_MAC], 0);
                    native_.PostMessageToJS(new JavaScriptSerializer().Serialize(response));
                    return;
                }

                wfdDevice = WiFiDirectDevice.FromIdAsync(deviceInfo.Id).AsTask().Result;
                wfdDevice.ConnectionStatusChanged += OnConnectionStatusChanged;

                var endpointPairs = wfdDevice.GetConnectionEndpointPairs();
                mConnected[extractMAC(wfdDevice.DeviceId)] = wfdDevice;

                response[TAG_DATA] = TAG_TRUE;
                response[TAG_SERVER_IP] = endpointPairs[0].RemoteHostName;
                mGroupOwner = false;
            } catch (Exception ex) {
                setError(response, ex.ToString(), ex.HResult);
                wfdDevice = null;
            }
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(response));
            if (wfdDevice != null)
                OnConnectionStatusChanged(wfdDevice, null);
        }

        void OnConnectionStatusChanged(WiFiDirectDevice sender, object arg) {
            JSONObject eventData = createEventData(EVENT_CONNECTION_CHANGED);
            ((JSONObject)eventData[TAG_DATA])[TAG_CONNECTED] = (sender.ConnectionStatus == WiFiDirectConnectionStatus.Connected) ? TAG_TRUE : TAG_FALSE;
            if (sender.ConnectionStatus == WiFiDirectConnectionStatus.Disconnected)
                mConnected.Remove(extractMAC(sender.DeviceId));
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(createEventData(EVENT_PEERS_CHANGED)));
            native_.PostMessageToJS(new JavaScriptSerializer().Serialize(eventData));
        }
        #endregion Inviting

        #region GettingInvited
        void advertise() {
            if (mAdvertiser == null) {
                mAdvertiser = new WiFiDirectAdvertisementPublisher();
            }

            if (mConnectionListener == null) {
                mConnectionListener = new WiFiDirectConnectionListener();
            }
            mConnectionListener.ConnectionRequested += OnConnectionRequested;
            mAdvertiser.Advertisement.ListenStateDiscoverability = WiFiDirectAdvertisementListenStateDiscoverability.Normal;
            mAdvertiser.StatusChanged += OnStatusChanged;
            mAdvertiser.Start();
        }

        void OnConnectionRequested(WiFiDirectConnectionListener sender, WiFiDirectConnectionRequestedEventArgs connectionEventArgs) {
            WiFiDirectDevice wfdDevice;
            try {
                var connectionRequest = connectionEventArgs.GetConnectionRequest();
                WiFiDirectConnectionParameters connectionParams = new WiFiDirectConnectionParameters();
                connectionParams.GroupOwnerIntent = 15;
                wfdDevice = WiFiDirectDevice.FromIdAsync(connectionRequest.DeviceInformation.Id, connectionParams).AsTask().Result;

                // Register for the ConnectionStatusChanged event handler
                wfdDevice.ConnectionStatusChanged += OnConnectionStatusChanged;
                mConnected[extractMAC(wfdDevice.DeviceId)] = wfdDevice;
                mGroupOwner = true;
            } catch (Exception ex) {
                Debug.WriteLine("Connect operation threw an exception: " + ex.Message);
                wfdDevice = null;
            }
            if (wfdDevice != null) {
                OnConnectionStatusChanged(wfdDevice, null);
            }
        }
        #endregion GettingInvited

        dynamic native_;
    }
}
