// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.xwalk.core.extension.ReflectionHelper.MemberType;

/*
 * The message handler for extensions.
 */

public class MessageHandler {
    private String TAG = "MessageHandler";

    public class Handler {
        MemberType type;
        String javaName;
        Object targetObject;
        ReflectionHelper reflection;

        public Handler(String javaName, MemberType type,
                Object object, ReflectionHelper reflection) {
            this.type = type;
            this.javaName = javaName;
            this.targetObject = object;
            this.reflection = reflection;
        }

        public Handler(String javaName, MemberType type, Object object) {
            this(javaName, type, object, null);
        }
    }
    private Map<String, Handler> mHandlers;

    public MessageHandler () {
        mHandlers = new HashMap<String, Handler>();
    }

    // The handler map from sourceHandler will be merged to current Map.
    public MessageHandler (MessageHandler sourceHandler) {
        mHandlers = new HashMap<String, Handler>();
        this.mHandlers.putAll(sourceHandler.mHandlers);
    }

    public void register(String jsName, String javaName,
            MemberType type, Object obj, ReflectionHelper reflection) {
        if (mHandlers.containsKey(jsName)) {
            Log.w(TAG, "Existing handler for " + jsName);
            return;
        }
        Handler handler = new Handler(javaName, type, obj, reflection);

        mHandlers.put(jsName, handler);
        return;
    }

    public void register(String jsName, String javaName,
            MemberType type, Object obj) {
        register(jsName, javaName, type, obj, null);
    }

    // The default type is a "method".
    public void register(String jsName, String javaName, Object obj) {
        register(jsName, javaName, MemberType.JS_METHOD, obj, null);
    }

    // The default javaName is same as the jsName.
    // Defualt type is "method".
    public void register(String jsName, Object obj) {
        register(jsName, jsName, MemberType.JS_METHOD, obj, null);
    }

    public Object handleMessage(MessageInfo info) {
        Object result = null;
        String jsName = info.getJsName();

        Handler handler = mHandlers.get(jsName);
        if (handler == null || handler.targetObject == null) {
            Log.w(TAG, "Cannot find handler for method " + jsName);
            return result;
        }
        Object obj = handler.targetObject;
        if (info.getExtension().isAutoJS() && handler.reflection != null) {
            try {
                result = handler.reflection.handleMessage(info, obj);
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
        } else {
            // This is not the generation case.
            // Currently, just support 'method'.
            Method method;
            try {
                method = obj.getClass().getMethod(handler.javaName, MessageInfo.class);
                result = method.invoke(obj, info);
            } catch (SecurityException | InvocationTargetException | NoSuchMethodException
                        | IllegalArgumentException | IllegalAccessException e) {
                Log.e(TAG, e.toString());
            }
        }
        return result;
    }
}
