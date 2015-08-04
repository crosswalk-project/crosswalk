// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.content.Intent;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * This class is to encapsulate the reflection detail of
 * invoking XWalkExtension class in the shared library APK.
 *
 * Each external extension should inherit this class and implements
 * below methods. It's created and registered by runtime side via the
 * configuration information in extensions-config.json.
 */
public class XWalkExtensionClient {
    // The unique name for this extension.
    protected String mName;

    // The JavaScript code stub. Will be injected to JS engine.
    protected String mJsApi;

    // The Entry points that will trigger the extension loading
    protected String[] mEntryPoints;

    // The context used by extensions.
    protected XWalkExtensionContextClient mExtensionContext;

    private Map<Integer, XWalkExtensionBindingObjectStore> instanceStores;

    // Reflection for JS stub generation
    protected ReflectionHelper reflection;

    /**
     * Constructor with the information of an extension.
     * @param name the extension name.
     * @param apiVersion the version of API.
     * @param jsApi the code stub of JavaScript for this extension.
     * @param context the extension context.
     */
    public XWalkExtensionClient(String name, String jsApi, XWalkExtensionContextClient context) {
        this(name, jsApi, null, context);
    }

    /**
     * Constructor with the information of an extension.
     * @param name the extension name.
     * @param apiVersion the version of API.
     * @param jsApi the code stub of JavaScript for this extension.
     * @param entryPoints Entry points are used when the extension needs to
     *                    have objects outside the namespace that is
     *                    implicitly created using its name.
     * @param context the extension context.
     */
    public XWalkExtensionClient(String name, String jsApi, String[] entryPoints, XWalkExtensionContextClient context) {
        assert (context != null);
        mName = name;
        mJsApi = jsApi;
        mEntryPoints = entryPoints;
        mExtensionContext = context;
        reflection = new ReflectionHelper(this.getClass());
        instanceStores = new HashMap<Integer, XWalkExtensionBindingObjectStore>();

        if (mJsApi == null || mJsApi.length() == 0) {
            mJsApi = new JsStubGenerator(reflection).generate();
            if (mJsApi == null || mJsApi.length() == 0) {
                Log.e("Extension-" + mName, "Can't generate JavaScript stub for this extension.");
                return;
            }
        }
        mExtensionContext.registerExtension(this);
    }

    /**
     * Get the unique name of extension.
     * @return the name of extension set from constructor.
     */
    public final String getExtensionName() {
        return mName;
    }

    /**
     * Get the JavaScript code stub.
     * @return the JavaScript code stub.
     */
    public final String getJsApi() {
        return mJsApi;
    }

    /**
     * Get the entry points list.
     * @return the JavaScript code stub.
     */
    public final String[] getEntryPoints() {
        return mEntryPoints;
    }

    /**
     * Called when this app is onStart.
     */
    public void onStart() {
    }

    /**
     * Called when this app is onResume.
     */
    public void onResume() {
    }

    /**
     * Called when this app is onPause.
     */
    public void onPause() {
    }

    /**
     * Called when this app is onStop.
     */
    public void onStop() {
    }

    /**
     * Called when this app is onDestroy.
     */
    public void onDestroy() {
    }

    /**
     * Called when this app is onNewIntent.
     */
    public void onNewIntent(Intent intent) {
    }

    /**
     * Tell extension that one activity exists so that it can know the result
     * of the exit code.
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
    }

    /**
     * Called when a new extension instance is created.
     */
    public void onInstanceCreated(int instanceID) {
        instanceStores.put(instanceID, new XWalkExtensionBindingObjectStore(this, instanceID));
    }

    /**
     * Called when a extension instance is destoryed.
     */
    public void onInstanceDestoryed(int instanceID) {
        instanceStores.remove(instanceID);
    }

    /**
     * Get the binding object store by instance ID.
     */
    public XWalkExtensionBindingObjectStore getInstanceStore(int instanceID) {
        return instanceStores.get(instanceID);
    }

    /**
     * JavaScript calls into Java code. The message is handled by
     * the extension implementation. The inherited classes should
     * override and add its implementation.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the message from JavaScript code.
     */
    public void onMessage(int extensionInstanceID, String message) {
        handleMessage(extensionInstanceID, message);
    }

    /**
     * Synchronized JavaScript calls into Java code. Similar to
     * onMessage. The only difference is it's a synchronized
     * message.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the message from JavaScript code.
     */
    public String onSyncMessage(int extensionInstanceID, String message) {
        Object result = handleMessage(extensionInstanceID, message);
        return (result != null) ? ReflectionHelper.objToJSON(result): "";
    }

    public ReflectionHelper getTargetReflect(String cName) {
        ReflectionHelper targetReflect = reflection.getConstructorReflection(cName);
        return (targetReflect != null)? targetReflect : reflection;
    }
    
    Object handleMessage(int extensionInstanceID, String message) {
        String TAG = "Extension-" + mName;
        try {
            JSONObject m = new JSONObject(message);
            String cmd = m.getString("cmd");
            int objectId = m.getInt("objectId");

            if (cmd.equals("jsObjectCollected")) {
                XWalkExtensionBindingObject obj =
                        getInstanceStore(extensionInstanceID).removeBindingObject(objectId);
                return null;
            } else if (cmd.equals("newInstance")) {
                XWalkExtensionBindingObject instance = (XWalkExtensionBindingObject)(reflection.invokeMethod(
                        this, extensionInstanceID, this, m.getString("name"), m.getJSONArray("args")));
                if (instance == null) return false;

                int newObjectId = m.getInt("bindingObjectId");
                return getInstanceStore(extensionInstanceID).addBindingObject(newObjectId, instance);
            } else {
                /*
                 * 1. message to the extension itself,  objectId:0,    cName:""
                 * 2. message to constructor,           objectId:0,    cName:[Its exported JS name]
                 * 3, message to object,                objectId:[>1], cName:[Its constructor's JS name]
                 */
                String cName = m.getString("constructorJsName");
                Object targetObj = null;
                if (objectId == 0) {
                    targetObj = (cName.length() == 0) ? this : null;
                } else {
                    targetObj = getInstanceStore(extensionInstanceID).getBindingObject(objectId);
                }
                return getTargetReflect(cName).handleMessage(this, extensionInstanceID, targetObj, m);
            }
        } catch (Exception e) {
            if (e instanceof JSONException) {
                Log.w(TAG, "Invalid message, error msg:\n" + e.toString());
            } else if (e instanceof IllegalArgumentException) {
                logJs(extensionInstanceID, e.toString(), "warn");
            } else {
                Log.w(TAG, "Failed to access member, error msg:\n" + e.toString());
            }
            e.printStackTrace();
        }
        return null;
    }

    protected Object getBindingStore(int instanceId) {
       return instanceStores.get(instanceId);
    }

    private void logJs(int instanceId, String msg, String level) {
        try {
            JSONObject msgOut = new JSONObject(); 
            msgOut.put("cmd", "error");
            msgOut.put("level", level);
            msgOut.put("msg", msg);
            postMessage(instanceId, msgOut.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void sendEvent(String type, Object event) {
        try {
            JSONObject msgOut = new JSONObject();
            msgOut.put("cmd", "onEvent");
            msgOut.put("type", type);
            msgOut.put("event", ReflectionHelper.objToJSON(event));
            broadcastMessage(msgOut.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Post messages to JavaScript via extension's context.
     * It's used by child classes to post message from Java side
     * to JavaScript side.
     * @param instanceID the ID of target extension instance.
     * @param message the message to be passed to Javascript.
     */
    public final void postMessage(int instanceID, String message) {
        mExtensionContext.postMessage(this, instanceID, message);
    }

    /**
     * Broadcast messages to JavaScript via extension's context.
     * It's used by child classes to broadcast message from Java side
     * to all JavaScript side instances of the extension.
     * @param message the message to be passed to Javascript.
     */
    public final void broadcastMessage(String message) {
        mExtensionContext.broadcastMessage(this, message);
    }
}
