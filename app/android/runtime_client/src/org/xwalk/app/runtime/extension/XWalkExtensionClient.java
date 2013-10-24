// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;

import java.lang.reflect.Method;
import java.util.StringTokenizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.xwalk.app.runtime.CrossPackageWrapper;

/**
 * This class is to encapsulate the reflection detail of
 * invoking XWalkExtension class in the shared library APK.
 *
 * Each external extension should inherit this class and implements
 * below methods. It's created and registered by runtime side via the
 * configuration information in extensions-config.json.
 */
public class XWalkExtensionClient extends CrossPackageWrapper {

    private final static String EXTENSION_CLASS_NAME = "org.xwalk.runtime.extension.XWalkExtensionClientImpl";
    private Object mInstance;
    private Method mGetExtensionName;
    private Method mGetJsApi;
    private Method mPostMessage;
    private Method mBroadcastMessage;

    protected XWalkExtensionContextClient mContext;

    public XWalkExtensionClient(String name, String jsApi, XWalkExtensionContextClient context) {
        super(context.getActivity(), EXTENSION_CLASS_NAME, null /* ExceptionHalder */, String.class, String.class,
                context.getInstance().getClass(), Object.class);
        mContext = context;
        mInstance = this.createInstance(name, jsApi, context.getInstance(), this);

        mGetExtensionName = lookupMethod("getExtensionName");
        mGetJsApi = lookupMethod("getJsApi");
        mPostMessage = lookupMethod("postMessage", int.class, String.class);
        mBroadcastMessage = lookupMethod("broadcastMessage", String.class);
    }

    /**
     * Get the extension name which is set when it's created.
     */
    public final String getExtensionName() {
        return (String)invokeMethod(mGetExtensionName, mInstance);
    }

    /**
     * Get the JavaScript stub code which is set when it's created.
     */
    public final String getJsApi() {
        return (String)invokeMethod(mGetJsApi, mInstance);
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
     * Called when this app is onDestroy.
     */
    public void onDestroy() {
    }

    /**
     * Tell extension that one activity exists so that it can know the result
     * of the exit code.
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
    }

    /**
     * JavaScript calls into Java code. The message is handled by
     * the extension implementation. The inherited classes should
     * override and add its implementation.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the message from JavaScript code.
     */
    public void onMessage(int extensionInstanceID, String message) {
    }

    /**
     * Synchronized JavaScript calls into Java code. Similar to
     * onMessage. The only difference is it's a synchronized
     * message.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the message from JavaScript code.
     */
    public String onSyncMessage(int extensionInstanceID, String message) {
        return "";
    }

    /**
     * Post messages to JavaScript via extension's context.
     * It's used by child classes to post message from Java side
     * to JavaScript side.
     * @param extensionInstanceID the ID of extension instance where the message came from.
     * @param message the message to be passed to Javascript.
     */
    public final void postMessage(int extensionInstanceID, String message) {
        invokeMethod(mPostMessage, mInstance, extensionInstanceID, message);
    }

    /**
     * Broadcast messages to JavaScript via extension's context.
     * It's used by child classes to broadcast message from Java side
     * to all JavaScript side instances of the extension.
     * @param message the message to be passed to Javascript.
     */
    public final void broadcastMessage(String message) {
        invokeMethod(mBroadcastMessage, mInstance, message);
    }
}
