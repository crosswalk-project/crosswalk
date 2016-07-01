// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;
import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.XWalkSettings;

/**
 * Test suite for XWalkSettings
 */
public class SettingsTest extends XWalkViewTestBase {
    private static final String USER_AGENT =
            "Chrome/44.0.2403.81 Crosswalk/15.44.376.0 Mobile Safari/537.36";
    private static final String LANGUAGE = "jp";
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mWebServer = TestWebServer.start();
    }

    @Override
    public void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    @SmallTest
    @Feature({"Settings"})
    public void testCacheMode() throws Throwable {
        final String htmlPath = "/testCacheMode.html";
        final String url = mWebServer.setResponse(htmlPath, "response", null);
        final String htmlNotInCachePath = "/testCacheMode-not-in-cache.html";
        final String urlNotInCache = mWebServer.setResponse(htmlNotInCachePath, "", null);

        clearCacheOnUiThread(true);
        assertEquals(XWalkSettings.LOAD_DEFAULT, getCacheMode());

        setCacheMode(XWalkSettings.LOAD_CACHE_ELSE_NETWORK);
        loadUrlSync(url);
        assertEquals(1, mWebServer.getRequestCount(htmlPath));
        loadUrlSync(url);
        assertEquals(1, mWebServer.getRequestCount(htmlPath));

        setCacheMode(XWalkSettings.LOAD_NO_CACHE);
        loadUrlSync(url);
        assertEquals(2, mWebServer.getRequestCount(htmlPath));
        loadUrlSync(url);
        assertEquals(3, mWebServer.getRequestCount(htmlPath));

        setCacheMode(XWalkSettings.LOAD_CACHE_ONLY);
        loadUrlSync(url);
        assertEquals(3, mWebServer.getRequestCount(htmlPath));
        loadUrlSync(url);
        assertEquals(3, mWebServer.getRequestCount(htmlPath));

        loadUrlSyncAndExpectError(urlNotInCache);
        assertEquals(0, mWebServer.getRequestCount(htmlNotInCachePath));
    }

    @SmallTest
    @Feature({"Settings"})
    // As our implementation of network loads blocking uses the same net::URLRequest settings, make
    // sure that setting cache mode doesn't accidentally enable network loads.  The reference
    // behaviour is that when network loads are blocked, setting cache mode has no effect.
    public void testCacheModeWithBlockedNetworkLoads() throws Throwable {
        final String htmlPath = "/testCacheModeWithBlockedNetworkLoads.html";
        final String url = mWebServer.setResponse(htmlPath, "response", null);

        clearCacheOnUiThread(true);
        assertEquals(XWalkSettings.LOAD_DEFAULT, getCacheMode());

        setBlockNetworkLoads(true);
        loadUrlSyncAndExpectError(url);
        assertEquals(0, mWebServer.getRequestCount(htmlPath));

        setCacheMode(XWalkSettings.LOAD_CACHE_ELSE_NETWORK);
        loadUrlSyncAndExpectError(url);
        assertEquals(0, mWebServer.getRequestCount(htmlPath));

        setCacheMode(XWalkSettings.LOAD_NO_CACHE);
        loadUrlSyncAndExpectError(url);
        assertEquals(0, mWebServer.getRequestCount(htmlPath));

        setCacheMode(XWalkSettings.LOAD_CACHE_ONLY);
        loadUrlSyncAndExpectError(url);
        assertEquals(0, mWebServer.getRequestCount(htmlPath));
    }

    @MediumTest
    @Feature({"Settings"})
    public void testUserAgentString() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkSettings settings = getXWalkView().getSettings();
                String defaultUserAgentString = settings.getUserAgentString();

                // Check that an attempt to set the default UA string to null or "" has no effect.
                settings.setUserAgentString(null);
                assertEquals(defaultUserAgentString, settings.getUserAgentString());
                settings.setUserAgentString("");
                assertEquals(defaultUserAgentString, settings.getUserAgentString());

                // Set a custom UA string, verify that it can be reset back to default.
                settings.setUserAgentString(USER_AGENT);
                assertEquals(USER_AGENT, settings.getUserAgentString());
                settings.setUserAgentString(null);
                assertEquals(defaultUserAgentString, settings.getUserAgentString());
            }
        });
    }

    @MediumTest
    @Feature({"Settings"})
    public void testAcceptLanguages() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkSettings settings = getXWalkView().getSettings();
                String defaultLanguages = settings.getAcceptLanguages();

                // Set a custom UA string, verify that it can be reset back to default.
                settings.setAcceptLanguages(LANGUAGE);
                assertEquals(LANGUAGE, settings.getAcceptLanguages());
                settings.setAcceptLanguages(defaultLanguages);
                assertEquals(defaultLanguages, settings.getAcceptLanguages());
            }
        });
    }

    @SmallTest
    @Feature({"Settings"})
    public void testSaveFormData() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkSettings settings = getXWalkView().getSettings();
                boolean defaultvalue = settings.getSaveFormData();

                settings.setSaveFormData(false);
                assertEquals(false, settings.getSaveFormData());
                settings.setSaveFormData(defaultvalue);
                assertEquals(defaultvalue, settings.getSaveFormData());
            }
        });
    }
}
