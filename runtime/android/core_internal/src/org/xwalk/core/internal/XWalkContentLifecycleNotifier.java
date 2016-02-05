// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.ObserverList;
import org.chromium.base.ThreadUtils;

/**
 * This class is intended to notify observers of the existence native instances of
 * xwalk_content. It receives a callback when native xwalk_content are created or
 * destroyed. Observers are notified when the first instance is created or the
 * last instance is destroyed.
 */
@JNINamespace("xwalk")
public class XWalkContentLifecycleNotifier {

    /**
     * Observer interface to be implemented by deriving xwalk lifecycle observers.
     */
    public static interface Observer {
        public void onFirstXWalkViewCreated();
        public void onLastXWalkViewDestroyed();
    }

    private static final ObserverList<Observer> sLifecycleObservers =
            new ObserverList<Observer>();
    private static int sNumXWalkViews = 0;

    private XWalkContentLifecycleNotifier() {}

    public static void addObserver(Observer observer) {
        sLifecycleObservers.addObserver(observer);
    }

    public static void removeObserver(Observer observer) {
        sLifecycleObservers.removeObserver(observer);
    }

    public static boolean hasXWalkViewInstances() {
        return sNumXWalkViews > 0;
    }

    // Called on UI thread.
    @CalledByNative
    private static void onXWalkViewCreated() {
        ThreadUtils.assertOnUiThread();
        assert sNumXWalkViews >= 0;
        sNumXWalkViews++;
        if (sNumXWalkViews == 1) {
            // first XWalkView created, notify observers.
            for (Observer observer : sLifecycleObservers) {
                observer.onFirstXWalkViewCreated();
            }
        }
    }

    // Called on UI thread.
    @CalledByNative
    private static void onXWalkViewDestroyed() {
        ThreadUtils.assertOnUiThread();
        assert sNumXWalkViews > 0;
        sNumXWalkViews--;
        if (sNumXWalkViews == 0) {
            // last XWalkView destroyed, notify observers.
            for (Observer observer : sLifecycleObservers) {
                observer.onLastXWalkViewDestroyed();
            }
        }
    }
}
