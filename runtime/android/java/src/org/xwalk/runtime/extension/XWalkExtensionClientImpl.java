// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * The public base class of xwalk external extensions. It acts a bridge class from
 * runtime to runtime client. The objects of class 'XWalkExtensionClient' in runtime
 * client side will be passed into this class because we need to call its methods.
 */
public class XWalkExtensionClientImpl extends XWalkExtension {

    private static final String TAG = XWalkExtensionClientImpl.class.getName();
    private Object mExtensionClient;
    private Method mOnMessage;
    private Method mOnSyncMessage;
    private Method mOnResume;
    private Method mOnPause;
    private Method mOnDestroy;
    private Method mOnActivityResult;

    public XWalkExtensionClientImpl(String name, String jsApi,
            XWalkExtensionContextWrapper context, Object extensionClient) {
        super(name, jsApi, context);

        mExtensionClient = extensionClient;
        mOnMessage = lookupMethod("onMessage", int.class, String.class);
        mOnSyncMessage = lookupMethod("onSyncMessage", int.class, String.class);
        mOnResume = lookupMethod("onResume");
        mOnPause = lookupMethod("onPause");
        mOnDestroy = lookupMethod("onDestroy");
        mOnActivityResult = lookupMethod("onActivityResult", int.class, int.class, Intent.class);
    }

    @Override
    public void onMessage(int extensionInstanceID, String message) {
        invokeMethod(mOnMessage, mExtensionClient, extensionInstanceID, message);
    }

    @Override
    public String onSyncMessage(int extensionInstanceID, String message) {
        return (String) invokeMethod(mOnSyncMessage, mExtensionClient, extensionInstanceID, message);
    }

    @Override
    public void onResume() {
        invokeMethod(mOnResume, mExtensionClient);
    }

    @Override
    public void onPause() {
        invokeMethod(mOnPause, mExtensionClient);
    }

    @Override
    public void onDestroy() {
        invokeMethod(mOnDestroy, mExtensionClient);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        invokeMethod(mOnActivityResult, mExtensionClient, requestCode, resultCode, data);
    }

    private Method lookupMethod(String method, Class<?>... parameters) {
        Class<?> clientClass = mExtensionClient.getClass();
        try {
            return clientClass.getMethod(method, parameters);
        } catch (NoSuchMethodException e) {
            handleException(e);
        }
        return null;
    }

    private static Object invokeMethod(Method method, Object instance, Object... parameters) {
        Object result = null;
        if (method != null) {
            try {
                result = method.invoke(instance, parameters);
            } catch (IllegalArgumentException e) {
                handleException(e);
            } catch (IllegalAccessException e) {
                handleException(e);
            } catch (InvocationTargetException e) {
                handleException(e);
            } catch (NullPointerException e) {
                handleException(e);
            }
        }
        return result;
    }

    private static void handleException(Exception e) {
        Log.e(TAG, "Error in calling methods of external extensions. " + e.toString());
        e.printStackTrace();
    }
}
