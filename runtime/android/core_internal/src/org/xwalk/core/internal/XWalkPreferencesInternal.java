// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

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
public class XWalkPreferencesInternal {
    private static HashMap<String, Boolean> sPrefMap = new HashMap<String, Boolean>();
    // Here we use WeakReference to make sure the KeyValueChangeListener instance
    // can be GC-ed to avoid memory leaking issue.
    private static ArrayList<WeakReference<KeyValueChangeListener> > sListeners =
            new ArrayList<WeakReference<KeyValueChangeListener> >();
    private static ReferenceQueue<KeyValueChangeListener> sRefQueue =
            new ReferenceQueue<KeyValueChangeListener>();

    /**
     * The key string to enable/disable remote debugging.
     */
    public static final String REMOTE_DEBUGGING = "remote-debugging";

    /**
     * The key string to enable/disable animatable XWalkViewInternal. Default value is
     * false.
     *
     * If this key is set to True, the XWalkViewInternal created by Crosswalk can be
     * transformed and animated. Internally, Crosswalk is alternatively using
     * TextureView as the backend of XWalkViewInternal.
     *
     * <a href="http://developer.android.com/reference/android/view/TextureView.html">
     * TextureView</a> is a kind of
     * <a href="http://developer.android.com/reference/android/view/View.html">
     * android.view.View</a> that is different from
     * <a href="http://developer.android.com/reference/android/view/SurfaceView.html">
     * SurfaceView</a>. Unlike SurfaceView, it can be resized, transformed and
     * animated. Once this key is set to True, all XWalkViewInternal will use TextureView
     * as the rendering target instead of SurfaceView. The downside of TextureView
     * is, it would consume more graphics memory than SurfaceView and may have
     * 1~3 extra frames of latency to display updates.
     *
     * Note this key MUST be set before creating the first XWalkViewInternal, otherwise
     * a RuntimeException will be thrown.
     */
    public static final String ANIMATABLE_XWALK_VIEW = "animatable-xwalk-view";

    static {
        sPrefMap.put(REMOTE_DEBUGGING, Boolean.FALSE);
        sPrefMap.put(ANIMATABLE_XWALK_VIEW, Boolean.FALSE);
    }

    /**
     * Set a preference value into Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @param enabled true if setting it as enabled.
     */
    public static synchronized void setValue(String key, boolean enabled) throws RuntimeException {
        checkKey(key);
        // If the listener list is not empty, we consider the preference is
        // loaded by Crosswalk and taken effect already.
        if (key == ANIMATABLE_XWALK_VIEW && !sListeners.isEmpty()) {
            throw new RuntimeException("Warning: the preference key " + key +
                    " can not be set if the preference is already loaded by Crosswalk");
        }
        if (sPrefMap.get(key) != enabled) {
            sPrefMap.put(key, new Boolean(enabled));
            onKeyValueChanged(key, enabled);
        }
    }

    /**
     * Get a preference value from Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @return true if it's enabled.
     */
    public static synchronized boolean getValue(String key) throws RuntimeException {
        checkKey(key);
        return sPrefMap.get(key);
    }

    // TODO(yongsheng): I believe this is needed?
    /*public static synchronized void setValue(String key, int value) throws RuntimeException {
    }*/

    static synchronized void load(KeyValueChangeListener listener) {
        // Load current settings for initialization of a listener implementor.
        for (Map.Entry<String, Boolean> entry : sPrefMap.entrySet()) {
            listener.onKeyValueChanged(entry.getKey(), entry.getValue());
        }

        registerListener(listener);
    }

    static synchronized void unload(KeyValueChangeListener listener) {
        unregisterListener(listener);
    }

    // Listen to value changes.
    interface KeyValueChangeListener {
        public void onKeyValueChanged(String key, boolean value);
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

    private static void onKeyValueChanged(String key, boolean enabled) {
        for (WeakReference<KeyValueChangeListener> weakListener : sListeners) {
            KeyValueChangeListener listener = weakListener.get();
            if (listener != null) listener.onKeyValueChanged(key, enabled);
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
