// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * This class represents the preferences and could be set by callers.
 * It is not thread-safe and must be called on the UI thread.
 * Afterwards, the preference could be read from all threads and can impact
 * all XWalkView instances.
 */
public final class XWalkPreferences {
    private static HashMap<String, Boolean> sPrefMap = new HashMap<String, Boolean>();
    private static ArrayList<KeyValueChangeListener> sListeners =
            new ArrayList<KeyValueChangeListener>();

    /**
     * The key string to enable/disable remote debugging.
     */
    public static final String REMOTE_DEBUGGING = "remote-debugging";

    static {
        sPrefMap.put(REMOTE_DEBUGGING, Boolean.FALSE);
    }

    /**
     * Set a preference value into Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @param enabled true if setting it as enabled.
     */
    public static synchronized void setValue(String key, boolean enabled) throws RuntimeException {
        checkKey(key);
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

    // Listen to value changes.
    interface KeyValueChangeListener {
        public void onKeyValueChanged(String key, boolean value);
    }

    private static synchronized void registerListener(KeyValueChangeListener listener) {
        sListeners.add(listener);
    }

    private static void onKeyValueChanged(String key, boolean enabled) {
        for (KeyValueChangeListener listener : sListeners) {
            listener.onKeyValueChanged(key, enabled);
        }
    }

    private static void checkKey(String key) throws RuntimeException {
        if (!sPrefMap.containsKey(key)) {
            throw new RuntimeException("Warning: the preference key " + key +
                    " is not supported by Crosswalk.");
        }
    }

}
