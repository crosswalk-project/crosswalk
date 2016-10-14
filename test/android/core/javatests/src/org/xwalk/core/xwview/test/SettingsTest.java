// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;
import android.test.suitebuilder.annotation.SmallTest;

import java.util.concurrent.Callable;

import org.chromium.base.test.util.UrlUtils;
import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.xwview.test.util.ImagePageGenerator;

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

    @MediumTest
    @Feature({"XWalkSettings", "Preferences"})
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

    @MediumTest
    @Feature({"XWalkSettings", "Preferences"})
    // As our implementation of network loads blocking uses the same net::URLRequest settings, make
    // sure that setting cache mode doesn't accidentally enable network loads. The reference
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
    @Feature({"XWalkSettings", "Preferences"})
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
    @Feature({"XWalkSettings", "Preferences"})
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
    @Feature({"XWalkSettings", "Preferences"})
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

    // The test verifies that JavaScript is enabled upon XWalkView
    // creation without accessing XWalkSettings. If the test passes,
    // it means that XWalkView-specific web preferences configuration
    // is applied on XWalkView creation.
    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testJavaScriptEnabledByDefault() throws Throwable {
        final String jsEnabledString = "JS has run";
        final String jsDisabledString = "JS has not run";
        final String testPageHtml =
                "<html><head><title>" + jsDisabledString + "</title>"
                + "</head><body onload=\"document.title='" + jsEnabledString
                + "';\"></body></html>";

        loadDataSync(testPageHtml, "text/html", false);
        assertEquals(jsEnabledString, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testJavaScriptEnabledWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsJavaScriptTestHelper(views.getView0(), views.getBridge0()),
                new XWalkSettingsJavaScriptTestHelper(views.getView1(), views.getBridge1()));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testJavaScriptEnabledDynamicWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsJavaScriptDynamicTestHelper(
                        views.getView0(), views.getBridge0()),
                new XWalkSettingsJavaScriptDynamicTestHelper(
                        views.getView1(), views.getBridge1()));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testJavaScriptPopupsWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsJavaScriptPopupsTestHelper(
                        views.getView0(), views.getBridge0()),
                new XWalkSettingsJavaScriptPopupsTestHelper(
                        views.getView1(), views.getBridge1()));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testDomStorageEnabledWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
            new XWalkSettingsDomStorageEnabledTestHelper(
                    views.getView0(), views.getBridge0()),
            new XWalkSettingsDomStorageEnabledTestHelper(
                    views.getView1(), views.getBridge1()));
    }

    // This setting is global in effect, across all XWalkView instances in a process.
    // So, we cannot combine these three tests into one, or using
    // runPerViewSettingsTest.
    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testDatabaseInitialValue() throws Throwable {
        ViewPair views = createViews();
        XWalkSettingsDatabaseTestHelper helper =
                new XWalkSettingsDatabaseTestHelper(
                        views.getView0(), views.getBridge0());
        helper.ensureSettingHasInitialValue();
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testDatabaseEnabled() throws Throwable {
        ViewPair views = createViews();
        XWalkSettingsDatabaseTestHelper helper =
                new XWalkSettingsDatabaseTestHelper(
                        views.getView0(), views.getBridge0());
        helper.setAlteredSettingValue();
        helper.ensureSettingHasAlteredValue();
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testDatabaseDisabled() throws Throwable {
        ViewPair views = createViews();
        XWalkSettingsDatabaseTestHelper helper =
                new XWalkSettingsDatabaseTestHelper(
                        views.getView0(), views.getBridge0());
        helper.setInitialSettingValue();
        helper.ensureSettingHasInitialValue();
    }

    // Test an assert URL (file:///android_asset/)
    @SmallTest
    @Feature({"XWalkSettings", "Navigation"})
    public void testAssetUrl() throws Throwable {
        // Note: this text needs to be kept in sync with the contents of the html file referenced
        // below.
        final String expectedTitle = "Asset File";
        loadUrlSync("file:///android_asset/www/asset_file.html");
        assertEquals(expectedTitle, getTitleOnUiThread());
    }

    // Test a resource URL (file:///android_res/).
    @SmallTest
    @Feature({"XWalkSettings", "Navigation"})
    public void testResourceUrl() throws Throwable {
        // Note: this text needs to be kept in sync with the contents of the html file referenced
        // below.
        final String expectedTitle = "Resource File";
        loadUrlSync("file:///android_res/raw/resource_file.html");
        assertEquals(expectedTitle, getTitleOnUiThread());
    }

    // Test that the file URL access toggle does not affect asset URLs.
    @SmallTest
    @Feature({"XWalkSettings", "Navigation"})
    public void testFileUrlAccessToggleDoesNotBlockAssetUrls() throws Throwable {
        // Note: this text needs to be kept in sync with the contents of the html file referenced
        // below.
        final String expectedTitle = "Asset File";
        setAllowFileAccess(false);
        loadUrlSync("file:///android_asset/www/asset_file.html");
        assertEquals(expectedTitle, getTitleOnUiThread());
    }

    // Test that the file URL access toggle does not affect resource URLs.
    @SmallTest
    @Feature({"XWalkSettings", "Navigation"})
    public void testFileUrlAccessToggleDoesNotBlockResourceUrls() throws Throwable {
        // Note: this text needs to be kept in sync with the contents of the html file referenced
        // below.
        final String expectedTitle = "Resource File";
        setAllowFileAccess(false);
        loadUrlSync("file:///android_res/raw/resource_file.html");
        assertEquals(expectedTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testFileUrlAccessWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsFileUrlAccessTestHelper(
                        views.getView0(), views.getBridge0(), 0),
                new XWalkSettingsFileUrlAccessTestHelper(
                        views.getView1(), views.getBridge1(), 1));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testContentUrlAccessWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsContentUrlAccessTestHelper(
                        views.getView0(), views.getBridge0(), 0),
                new XWalkSettingsContentUrlAccessTestHelper(
                        views.getView1(), views.getBridge1(), 1));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences", "Navigation"})
    public void testContentUrlFromFileWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsContentUrlAccessFromFileTestHelper(
                        views.getView0(), views.getBridge0(), 0),
                new XWalkSettingsContentUrlAccessFromFileTestHelper(
                        views.getView1(), views.getBridge1(), 1));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testUniversalAccessFromFilesWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsUniversalAccessFromFilesTestHelper(
                        views.getView0(), views.getBridge0()),
                new XWalkSettingsUniversalAccessFromFilesTestHelper(
                        views.getView1(), views.getBridge1()));
    }

    // This test verifies that local image resources can be loaded from file:
    // URLs regardless of file access state.
    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testFileAccessFromFilesImage() throws Throwable {
        final String testFile = "xwalkview/image_access.html";
        assertFileIsReadable(UrlUtils.getTestFilePath(testFile));
        final String imageContainerUrl = UrlUtils.getTestFileUrl(testFile);
        final String imageHeight = "145";

        setAllowUniversalAccessFromFileURLs(false);
        setAllowFileAccessFromFileURLs(false);
        loadUrlSync(imageContainerUrl);
        assertEquals(imageHeight, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testFileAccessFromFilesIframeWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsFileAccessFromFilesIframeTestHelper(
                        views.getView0(), views.getBridge0()),
                new XWalkSettingsFileAccessFromFilesIframeTestHelper(
                        views.getView1(), views.getBridge1()));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testFileAccessFromFilesXhrWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsFileAccessFromFilesXhrTestHelper(
                        views.getView0(), views.getBridge0()),
                new XWalkSettingsFileAccessFromFilesXhrTestHelper(
                        views.getView1(), views.getBridge1()));
    }

    // The test verifies that after changing the LoadsImagesAutomatically
    // setting value from false to true previously skipped images are
    // automatically loaded.
    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testLoadsImagesAutomaticallyNoPageReload() throws Throwable {
        ImagePageGenerator generator = new ImagePageGenerator(0, false);
        setLoadsImagesAutomatically(false);
        loadDataSync(generator.getPageSource(), "text/html", false);
        assertEquals(ImagePageGenerator.IMAGE_NOT_LOADED_STRING,
                getTitleOnUiThread());
        setLoadsImagesAutomatically(true);
        pollInstrumentationThread(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return !ImagePageGenerator.IMAGE_NOT_LOADED_STRING.equals(
                        getTitleOnUiThread());
            }
        });
        assertEquals(ImagePageGenerator.IMAGE_LOADED_STRING, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testLoadsImagesAutomaticallyWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsLoadImagesAutomaticallyTestHelper(
                    views.getView0(), views.getBridge0(), new ImagePageGenerator(0, true)),
                new XWalkSettingsLoadImagesAutomaticallyTestHelper(
                    views.getView1(), views.getBridge1(), new ImagePageGenerator(1, true)));
    }

    @SmallTest
    @Feature({"XWalkSettings", "Preferences"})
    public void testBlockNetworkImagesWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsBlockNetworkImageHelper(
                        views.getView0(),
                        views.getBridge0(),
                        mWebServer,
                        new ImagePageGenerator(0, true)),
                new XWalkSettingsBlockNetworkImageHelper(
                        views.getView1(),
                        views.getBridge1(),
                        mWebServer,
                        new ImagePageGenerator(1, true)));
    }
}
