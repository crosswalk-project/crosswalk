// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.content.Context;
import android.content.Intent;

/**
 * The public base class of xwalk extensions. Each extension should inherit
 * this class and implement its interfaces. Note that it's for every extensions.
 * For internal extensions, each one should base on it directly. For external
 * extensions, there'll be a bridge class in the runtime client side.
 */
public abstract class XWalkExtension {
    // The unique name for this extension.
    protected String mName;

    // The JavaScript code stub. Will be injected to JS engine.
    protected String mJsApi;

    // The context used by extensions.
    protected XWalkExtensionContext mExtensionContext;

    // The ID for registration.
    private Object mRegisteredId;

    /**
     * Constructor with the information of an extension.
     * @param name the extension name.
     * @param apiVersion the version of API.
     * @param jsApi the code stub of JavaScript for this extension.
     * @param context the extension context.
     */
    public XWalkExtension(String name, String jsApi, XWalkExtensionContext context) {
        mName = name;
        mJsApi = jsApi;
        mExtensionContext = context;
        mRegisteredId = mExtensionContext.registerExtension(this);
    }

    /**
     * Get the unique name of extension.
     * @return the name of extension set from constructor.
     */
    public String getExtensionName() {
        return mName;
    }

    /**
     * Get the JavaScript code stub.
     * @return the JavaScript code stub.
     */
    public String getJsApi() {
        return mJsApi;
    }

    /**
     * JavaScript call into Java code. The message contains
     * the JavaScript function name and parameters.
     * The message format should be like below:
     * @param message the message from JavaScript code.
     */
    public abstract void onMessage(String message);

    /**
     * Synchronized JavaScript call into Java code. Similar to
     * onMessage. The only difference is it's a synchronized
     * message.
     * @param message the message from JavaScript code.
     */
    public String onSyncMessage(String message) {
        return null;
    }

    /**
     * Post messages to JavaScript via extension's context.
     * @param message the message to be passed to Javascript.
     */
    public void postMessage(String message) {
        mExtensionContext.postMessage(this, message);
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
     * Tell extension that one activity exists so that it can know the result code
     * of the exit code.
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
    }

    /**
     * Get the registered ID.
     * @return the registered ID object.
     */
    public Object getRegisteredId() {
        return mRegisteredId;
    }
}
