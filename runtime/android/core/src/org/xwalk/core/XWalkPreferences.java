// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.xwalk.core.internal.XWalkPreferencesInternal;

/**
 * This class represents the preferences and could be set by callers.
 * It is not thread-safe and must be called on the UI thread.
 * Afterwards, the preference could be read from all threads and can impact
 * all XWalkView instances.
 */
public final class XWalkPreferences extends XWalkPreferencesInternal {
    /**
     * The key string to enable/disable remote debugging.
     * @since 1.0
     */
    public static final String REMOTE_DEBUGGING = "remote-debugging";

    /**
     * The key string to enable/disable animatable XWalkView. Default value is
     * false.
     *
     * If this key is set to True, the XWalkView created by Crosswalk can be
     * transformed and animated. Internally, Crosswalk is alternatively using
     * TextureView as the backend of XWalkView.
     *
     * <a href="http://developer.android.com/reference/android/view/TextureView.html">
     * TextureView</a> is a kind of
     * <a href="http://developer.android.com/reference/android/view/View.html">
     * android.view.View</a> that is different from
     * <a href="http://developer.android.com/reference/android/view/SurfaceView.html">
     * SurfaceView</a>. Unlike SurfaceView, it can be resized, transformed and
     * animated. Once this key is set to True, all XWalkView will use TextureView
     * as the rendering target instead of SurfaceView. The downside of TextureView
     * is, it would consume more graphics memory than SurfaceView and may have
     * 1~3 extra frames of latency to display updates.
     *
     * Note this key MUST be set before creating the first XWalkView, otherwise
     * a RuntimeException will be thrown.
     * @since 2.0
     */
    public static final String ANIMATABLE_XWALK_VIEW = "animatable-xwalk-view";

    /**
     * Set a preference value into Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @param enabled true if setting it as enabled.
     * @since 1.0
     */
    public static synchronized void setValue(String key, boolean enabled) throws RuntimeException {
        XWalkPreferencesInternal.setValue(key, enabled);
    }

    /**
     * Get a preference value from Crosswalk. An exception will be thrown if
     * the key for the preference is not valid.
     * @param key the string name of the key.
     * @return true if it's enabled.
     * @since 1.0
     */
    public static synchronized boolean getValue(String key) throws RuntimeException {
        return XWalkPreferencesInternal.getValue(key);
    }
}
