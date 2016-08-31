// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.MediumTest;
import android.util.Log;
import android.test.TouchUtils;
import android.test.InstrumentationTestCase;

import java.util.concurrent.TimeUnit;

import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;

import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.internal.XWalkViewInternal;

/**
 * Renderer responsiveness tests:
 *
 * Internally, a hang monitor timer will start for each renderer when there is
 * an input event sent to the renderer. If the ACK for handling the input event
 * is not received in 30 seconds, the renderer is deemed to be unresponsive to
 * user interaction.
 */
public class RendererResponsivenessTest extends XWalkViewInternalTestBase {
    private OnRendererResponsivenessHelper responsiveHelper = new OnRendererResponsivenessHelper();
    private OnRendererResponsivenessHelper unresponsiveHelper = new OnRendererResponsivenessHelper();

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    //@Feature({"RendererResponsivenessTest"})
    //@MediumTest
    @DisabledTest
    public void testRendererUnresponsive() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkClient(new XWalkClient(getXWalkView()) {
                    @Override
                    public void onRendererUnresponsive(XWalkViewInternal view) {
                        unresponsiveHelper.notifyCalled(view);
                    }
                });
            }
        });

        loadAssetFile("renderHung.html");

        int currentCallCount = unresponsiveHelper.getCallCount();
        getXWalkView().evaluateJavascript("deadLoopForever();", null);

        /**
         * Send an input event to xwalk view. Internally, if no ACK message is received
         * for handling this input event in 30 seconds, onRendererUnresponsive callback
         * would be called.
         */
        TouchUtils.clickView(this, getXWalkView());

        /**
         * The timeout for responsiveness checking is 30 seconds, so here the timeout
         * for callback is set to 40 seconds to ensure the onRendererResponsive
         * to be called.
         */
        unresponsiveHelper.waitForCallback(currentCallCount, 1, 40, TimeUnit.SECONDS);
        assertEquals(getXWalkView(), unresponsiveHelper.getXWalkView());
    }

    //@Feature({"RendererResponsivenessTest"})
    //@MediumTest
    @DisabledTest
    public void testRendererResponsiveAgain() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkClient(new XWalkClient(getXWalkView()) {
                    @Override
                    public void onRendererResponsive(XWalkViewInternal view) {
                        responsiveHelper.notifyCalled(view);
                    }
                });
            }
        });

        loadAssetFile("renderHung.html");

        int currentCallCount = responsiveHelper.getCallCount();
        getXWalkView().evaluateJavascript("deadLoopFor40secs();", null);

        /**
         * Send an input event to start the hung monitor.
         */
        TouchUtils.clickView(this, getXWalkView());

        /**
         * Wait for onRendererResponsive to called. The dead loop is designed to run
         * 40 seconds, and here we wait for 50 seconds to ensure it has enough time to
         * call onRendererResponsive callback.
         */
        responsiveHelper.waitForCallback(currentCallCount, 1, 50, TimeUnit.SECONDS);
        assertEquals(getXWalkView(), responsiveHelper.getXWalkView());
    }

    private final class OnRendererResponsivenessHelper extends CallbackHelper {
        private XWalkViewInternal mView;

        public void notifyCalled(XWalkViewInternal view) {
            mView = view;
            notifyCalled();
        }
        public XWalkViewInternal getXWalkView() {
            assert getCallCount() > 0;
            return mView;
        }
    }
}
