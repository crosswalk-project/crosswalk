// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.content.Intent;
import android.os.Bundle;
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
 * configuration information in your myextension.json.
 */
public class XWalkExternalExtension {
    // The unique name for this extension.
    protected String mName;

    // The JavaScript code stub. Will be injected to JS engine.
    protected String mJsApi;

    // The Entry points that will trigger the extension loading
    protected String[] mEntryPoints;

    private Map<Integer, ExtensionInstanceHelper> instanceHelpers;

    // The context used by extensions.
    protected XWalkExtensionContextClient mExtensionContext;

    // Reflection for JS stub generation
    protected ReflectionHelper mReflection;

    protected boolean useJsStubGeneration;
    protected MessageHandler mHandler;

    /**
     * Constructor with the information of an extension.
     * @param name the extension name.
     * @param apiVersion the version of API.
     * @param jsApi the code stub of JavaScript for this extension.
     * @param context the extension context.
     */
    public XWalkExternalExtension(String name, String jsApi, XWalkExtensionContextClient context) {
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
    public XWalkExternalExtension(String name, String jsApi, String[] entryPoints, XWalkExtensionContextClient context) {
        assert (context != null);
        mName = name;
        mJsApi = jsApi;
        mEntryPoints = entryPoints;
        mExtensionContext = context;
        instanceHelpers = new HashMap<Integer, ExtensionInstanceHelper>();
        mHandler = new MessageHandler();

        if (mJsApi == null || mJsApi.length() == 0) {
            useJsStubGeneration = true;
            mReflection = new ReflectionHelper(this.getClass());
            mJsApi = new JsStubGenerator(mReflection).generate();
            if (mJsApi == null || mJsApi.length() == 0) {
                Log.e("Extension-" + mName, "Can't generate JavaScript stub for this extension.");
                return;
            }
        } else {
            mReflection = null;
            useJsStubGeneration = false;
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
     * Called when a new extension instance is created.
     */
    public void onInstanceCreated(int instanceID) {
        instanceHelpers.put(instanceID,
                new ExtensionInstanceHelper(this, instanceID));
    }

    /**
     * Called when a extension instance is destroyed.
     */
    public void onInstanceDestroyed(int instanceID) {
        instanceHelpers.remove(instanceID);
    }

    public boolean isAutoJS() {
        return useJsStubGeneration;
    }

    /**
     * JavaScript calls into Java code. The message is handled by
     * the extension implementation. The inherited classes should
     * override and add its implementation.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the message from JavaScript code.
     */
    public void onMessage(int extensionInstanceID, String message) {
        if (useJsStubGeneration) {
            getInstanceHelper(extensionInstanceID).handleMessage(message);
        }
    }

    /**
     * JavaScript calls into Java code. The message is handled by
     * the extension implementation. The inherited classes should
     * override and add its implementation.
     * JavaScript wraps the binary message into an ArrayBuffer.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the binary message from JavaScript code.
     */
    public void onBinaryMessage(int extensionInstanceID, byte[] message) {
        if (useJsStubGeneration) {
            getInstanceHelper(extensionInstanceID).handleMessage(message);
        }
    }

    /**
     * Synchronized JavaScript calls into Java code. Similar to
     * onMessage. The only difference is it's a synchronized
     * message.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the message from JavaScript code.
     */
    public String onSyncMessage(int extensionInstanceID, String message) {
        Object result = null;
        if (useJsStubGeneration) {
            result = getInstanceHelper(extensionInstanceID).handleMessage(message);
        }
        return (result != null) ? ReflectionHelper.objToJSON(result): "";
    }

    public ReflectionHelper getReflection() {
        return mReflection;
    }

    public MessageHandler getMessageHandler() {
        return mHandler;
    }

    public ReflectionHelper getTargetReflect(String cName) {
        ReflectionHelper targetReflect = mReflection.getConstructorReflection(cName);
        return (targetReflect != null) ? targetReflect : mReflection;
    }

    protected ExtensionInstanceHelper getInstanceHelper(int instanceId) {
        return instanceHelpers.get(instanceId);
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
     * Post binary messages to JavaScript via extension's context.
     * It's used by child classes to post message from Java side
     * to JavaScript side.
     * JavaScript recevies the binary message in an ArrayBuffer.
     * @param instanceID the ID of target extension instance.
     * @param message the binary message to be passed to Javascript.
     */
    public final void postBinaryMessage(int instanceID, byte[] message) {
        mExtensionContext.postBinaryMessage(this, instanceID, message);
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
