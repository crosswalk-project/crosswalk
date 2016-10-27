// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.util.Log;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * This class represents the preferences and could be set by callers.
 * It is not thread-safe and must be called on the UI thread.
 * Afterwards, the preference could be read from all threads and can impact
 * all XWalkViewInternal instances.
 */
@XWalkAPI(noInstance = true)
public class XWalkPreferencesInternal {
    private static final String TAG = "XWalkPreferences";

    static class PreferenceValue {
        static final int PREFERENCE_TYPE_BOOLEAN = 1;
        static final int PREFERENCE_TYPE_INTEGER = 2;
        static final int PREFERENCE_TYPE_STRING = 3;

        int mType;
        Object mValue;

        PreferenceValue(boolean value) {
            mType = PREFERENCE_TYPE_BOOLEAN;
            mValue = value;
        }

        PreferenceValue(int value) {
            mType = PREFERENCE_TYPE_INTEGER;
            mValue = value;
        }

        PreferenceValue(String value) {
            mType = PREFERENCE_TYPE_STRING;
            mValue = value;
        }

        int getType() {
            return mType;
        }

        boolean getBooleanValue() {
            if (mType != PREFERENCE_TYPE_BOOLEAN) return false;
            return (Boolean) mValue;
        }

        int getIntegerValue() {
            if (mType != PREFERENCE_TYPE_INTEGER) return -1;
            return (Integer) mValue;
        }

        String getStringValue() {
            if (mType != PREFERENCE_TYPE_STRING) return null;
            return (String) mValue;
        }
    }

    private static HashMap<String, PreferenceValue> sPrefMap = new HashMap<String, PreferenceValue>();
    // Here we use WeakReference to make sure the KeyValueChangeListener instance
    // can be GC-ed to avoid memory leaking issue.
    private static ArrayList<WeakReference<KeyValueChangeListener> > sListeners =
            new ArrayList<WeakReference<KeyValueChangeListener> >();
    private static ReferenceQueue<KeyValueChangeListener> sRefQueue =
            new ReferenceQueue<KeyValueChangeListener>();

    /**
     * The key string to enable/disable remote debugging.
     * @since 1.0
     */
    @XWalkAPI
    public static final String REMOTE_DEBUGGING = "remote-debugging";

    /**
     * The key string to enable/disable animatable XWalkViewInternal. Default value is
     * false.
     *
     * If this key is set to false, then SurfaceView will be created internally as the
     * rendering backend.
     * If this key is set to true, the XWalkViewInternal created by Crosswalk can be
     * transformed and animated. Internally, Crosswalk is alternatively using
     * TextureView as the backend of XWalkViewInternal.
     *
     * <a href="http://developer.android.com/reference/android/view/TextureView.html">
     * TextureView</a> is a kind of
     * <a href="http://developer.android.com/reference/android/view/View.html">
     * android.view.View</a> that is different from
     * <a href="http://developer.android.com/reference/android/view/SurfaceView.html">
     * SurfaceView</a>. Unlike SurfaceView, it can be resized, transformed and
     * animated. Once this key is set to true, all XWalkViewInternal will use TextureView
     * as the rendering target instead of SurfaceView. The downside of TextureView
     * is, it would consume more graphics memory than SurfaceView and may have
     * 1~3 extra frames of latency to display updates.
     *
     * Note this key MUST be set before creating the first XWalkViewInternal, otherwise
     * a RuntimeException will be thrown.
     *
     * @since 2.0
     */
    @XWalkAPI
    public static final String ANIMATABLE_XWALK_VIEW = "animatable-xwalk-view";

    /**
     * The key string to allow/disallow javascript to open
     * window automatically.
     * @since 3.0
     */
    @XWalkAPI
    public static final String JAVASCRIPT_CAN_OPEN_WINDOW =
            "javascript-can-open-window";

    /**
     * The key string to allow/disallow having universal access
     * from file origin.
     * @since 3.0
     */
    @XWalkAPI
    public static final String ALLOW_UNIVERSAL_ACCESS_FROM_FILE =
            "allow-universal-access-from-file";

    /**
     * The key string to enable/disable multiple windows.
     * @since 3.0
     */
    @XWalkAPI
    public static final String SUPPORT_MULTIPLE_WINDOWS =
            "support-multiple-windows";

    /**
     * The key string to set xwalk profile name.
     * User data will be kept separated for different profiles.
     * Profile needs to be set before any XWalkView instance created.
     * @since 3.0
     */
    @XWalkAPI
    public static final String PROFILE_NAME = "profile-name";

    /**
     * The key string to enable/disable spatial navigation like a TV controller.
     * @since 6.0
     */
    @XWalkAPI
    public static final String SPATIAL_NAVIGATION = "enable-spatial-navigation";

    /*
     * The key string to enable/disable website's "theme-color" attribute.
     * Default is true, and it only works on Android Lollipop or later.
     * @since 6.0
     */
    @XWalkAPI
    public static final String ENABLE_THEME_COLOR = "enable-theme-color";

    /**
     * The key string to enable/disable javascript.
     * @since 7.0
     */
    @XWalkAPI
    public static final String ENABLE_JAVASCRIPT = "enable-javascript";

    /**
     * The key string to enable/disable xwalk extensions.
     * @since 7.0
     */
    @XWalkAPI
    public static final String ENABLE_EXTENSIONS = "enable-extensions";

    static {
        sPrefMap.put(REMOTE_DEBUGGING, new PreferenceValue(false));
        sPrefMap.put(ANIMATABLE_XWALK_VIEW, new PreferenceValue(false));
        sPrefMap.put(ENABLE_JAVASCRIPT, new PreferenceValue(true));
        sPrefMap.put(JAVASCRIPT_CAN_OPEN_WINDOW, new PreferenceValue(true));
        sPrefMap.put(
                ALLOW_UNIVERSAL_ACCESS_FROM_FILE, new PreferenceValue(false));
        sPrefMap.put(SUPPORT_MULTIPLE_WINDOWS, new PreferenceValue(false));
        sPrefMap.put(ENABLE_EXTENSIONS, new PreferenceValue(true));
        sPrefMap.put(PROFILE_NAME, new PreferenceValue("Default"));
        sPrefMap.put(SPATIAL_NAVIGATION, new PreferenceValue(true));
        sPrefMap.put(ENABLE_THEME_COLOR, new PreferenceValue(true));
    }

    /**
     * Set a boolean preference value into Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @param enabled true if setting it as enabled.
     * @since 1.0
     */
    @XWalkAPI(reservable = true)
    public static synchronized void setValue(String key, boolean enabled) throws RuntimeException {
        checkKey(key);
        // If the listener list is not empty, we consider the preference is
        // loaded by Crosswalk and taken effect already.
        if (key == ANIMATABLE_XWALK_VIEW && !sListeners.isEmpty()) {
            Log.d(TAG, "ANIMATABLE_XWALK_VIEW is not effective to existing XWalkView objects");
        }
        if (sPrefMap.get(key).getBooleanValue() != enabled) {
            PreferenceValue v = new PreferenceValue(enabled);
            sPrefMap.put(key, v);
            onKeyValueChanged(key, v);
        }
    }

    /**
     * Set an integer preference value into Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @param value the integer value.
     * @since 3.0
     */
    @XWalkAPI(reservable = true)
    public static synchronized void setValue(String key, int value) throws RuntimeException {
        checkKey(key);
        // If the listener list is not empty, we consider the preference is
        // loaded by Crosswalk and taken effect already.
        if (key == ANIMATABLE_XWALK_VIEW && !sListeners.isEmpty()) {
            Log.d(TAG, "ANIMATABLE_XWALK_VIEW is not effective to existing XWalkView objects");
        }
        if (sPrefMap.get(key).getIntegerValue() != value) {
            PreferenceValue v = new PreferenceValue(value);
            sPrefMap.put(key, v);
            onKeyValueChanged(key, v);
        }
    }

    /**
     * Set a string preference value into Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @param value the string value.
     * @since 3.0
     */
    @XWalkAPI(reservable = true)
    public static synchronized void setValue(String key, String value) throws RuntimeException {
        checkKey(key);
        // If the listener list is not empty, we consider the preference is
        // loaded by Crosswalk and taken effect already.
        if (key == ANIMATABLE_XWALK_VIEW && !sListeners.isEmpty()) {
            Log.d(TAG, "ANIMATABLE_XWALK_VIEW is not effective to existing XWalkView objects");
        }
        if (value != null && !value.equals(sPrefMap.get(key).getStringValue())) {
            PreferenceValue v = new PreferenceValue(value);
            sPrefMap.put(key, v);
            onKeyValueChanged(key, v);
        }
    }

    /**
     * Get a boolean preference value from Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @return true if it's enabled.
     * @since 1.0
     * @deprecated
     */
    @XWalkAPI
    public static synchronized boolean getValue(String key) throws RuntimeException {
        checkKey(key);
        return sPrefMap.get(key).getBooleanValue();
    }

    /**
     * Get a boolean preference value from Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @return true if it's enabled.
     * @since 3.0
     */
    @XWalkAPI
    public static synchronized boolean getBooleanValue(String key) throws RuntimeException {
        checkKey(key);
        return sPrefMap.get(key).getBooleanValue();
    }

    /**
     * Get a int preference value from Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @return the integer value.
     * @since 3.0
     */
    @XWalkAPI
    public static synchronized int getIntegerValue(String key) throws RuntimeException {
        checkKey(key);
        return sPrefMap.get(key).getIntegerValue();
    }

    /**
     * Get a string preference value from Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @return the string value.
     * @since 3.0
     */
    @XWalkAPI
    public static synchronized String getStringValue(String key) throws RuntimeException {
        checkKey(key);
        return sPrefMap.get(key).getStringValue();
    }

    static synchronized void load(KeyValueChangeListener listener) {
        // Load current settings for initialization of a listener implementor.
        for (Map.Entry<String, PreferenceValue> entry : sPrefMap.entrySet()) {
            listener.onKeyValueChanged(entry.getKey(), entry.getValue());
        }

        registerListener(listener);
    }

    static synchronized void unload(KeyValueChangeListener listener) {
        unregisterListener(listener);
    }

    // Listen to value changes.
    interface KeyValueChangeListener {
        public void onKeyValueChanged(String key, PreferenceValue value);
    }

    private static synchronized void registerListener(KeyValueChangeListener listener) {
        removeEnqueuedReference();
        WeakReference<KeyValueChangeListener> weakListener =
                new WeakReference<KeyValueChangeListener>(listener, sRefQueue);
        sListeners.add(weakListener);
    }

    private static synchronized void unregisterListener(KeyValueChangeListener listener) {
        removeEnqueuedReference();
        for (WeakReference<KeyValueChangeListener> weakListener : sListeners) {
            if (weakListener.get() == listener) {
                sListeners.remove(weakListener);
                break;
            }
        }
    }

    private static void onKeyValueChanged(String key, PreferenceValue value) {
        for (WeakReference<KeyValueChangeListener> weakListener : sListeners) {
            KeyValueChangeListener listener = weakListener.get();
            if (listener != null) listener.onKeyValueChanged(key, value);
        }
    }

    private static void checkKey(String key) throws RuntimeException {
        removeEnqueuedReference();
        if (!sPrefMap.containsKey(key)) {
            throw new RuntimeException("Warning: the preference key " + key +
                    " is not supported by Crosswalk.");
        }
    }

    /**
     * Internal method to keep track of weak references and remove the enqueued
     * references from listener list by polling the reference queue.
     */
    @SuppressWarnings("unchecked")
    private static void removeEnqueuedReference() {
        WeakReference<KeyValueChangeListener> toRemove;
        while ((toRemove = (WeakReference<KeyValueChangeListener>) sRefQueue.poll()) != null) {
            sListeners.remove(toRemove);
        }
    }
}
