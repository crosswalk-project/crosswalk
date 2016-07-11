// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Message;
import android.test.suitebuilder.annotation.LargeTest;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import android.view.KeyEvent;

import java.io.InputStream;
import java.net.URL;
import java.util.concurrent.Callable;


import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.internal.XWalkHitTestResultInternal;
import org.xwalk.core.internal.XWalkViewInternal;
import org.xwalk.core.internal.xwview.test.util.CommonResources;

public class HitTestTest extends XWalkViewInternalTestBase {
    private TestWebServer mWebServer;
    private XWalkViewInternal mXWalkViewInternal;
    private ContentViewCore mContentViewCore;
    private static final String HREF = "http://foo/";
    private static final String ANCHOR_TEXT = "anchor text";

    @Override
    public void setUp() throws Exception{
      super.setUp();

      mXWalkViewInternal = getXWalkView();
      mContentViewCore = getContentViewCore();
      mWebServer = TestWebServer.start();
      final String imagePath = "/" + CommonResources.TEST_IMAGE_FILENAME;
      mWebServer.setResponseBase64(imagePath,
              CommonResources.FAVICON_DATA_BASE64, CommonResources.getImagePngHeaders(true));
    }

    @Override
    public void tearDown() throws Exception {
        if (mWebServer != null) {
            mWebServer.shutdown();
        }
        super.tearDown();
    }

    private void setServerResponseAndLoad(String response) throws Throwable {
        String url = mWebServer.setResponse("/hittest.html", response, null);
        loadUrlSync(url);
    }

    private static String fullPageLink(String href, String anchorText) {
        return CommonResources.makeHtmlPageFrom("", "<a class=\"full_view\" href=\""
                + href + "\" " + "onclick=\"return false;\">" + anchorText + "</a>");
    }

    private void pollForHitTestDataOnUiThread(
            final XWalkHitTestResultInternal.type expectedType, final String expectedExtra) throws Throwable {
        pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                XWalkHitTestResultInternal data = mXWalkViewInternal.getHitTestResult();
                return expectedType == data.getType()
                        && stringEquals(expectedExtra, data.getExtra());
            }
        });
    }

    private void simulateTabDownUpOnUiThread() throws Throwable {
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
              mContentViewCore.dispatchKeyEvent(
                  new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_TAB));
              mContentViewCore.dispatchKeyEvent(
                  new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_TAB));
            }
        });
    }

    private void simulateInput(boolean byTouch) throws Throwable {
        // Send a touch click event if byTouch is true. Otherwise, send a TAB
        // key event to change the focused element of the page.
        if (byTouch) {
            XWalkViewInternalTestTouchUtils.simulateTouchCenterOfView(mXWalkViewInternal);
        } else {
            simulateTabDownUpOnUiThread();
        }
    }

    private static boolean stringEquals(String a, String b) {
        return a == null ? b == null : a.equals(b);
    }

    private void srcAnchorTypeTestBody(boolean byTouch) throws Throwable {
        String page = fullPageLink(HREF, ANCHOR_TEXT);
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.SRC_ANCHOR_TYPE, HREF);
        // TODO(Chuan): Add requestFocusNodeHref test after implementation
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcAnchorType() throws Throwable {
        srcAnchorTypeTestBody(true);
    }

    private void blankHrefTestBody(boolean byTouch) throws Throwable {
        String fullPath = mWebServer.getResponseUrl("/hittest.html");
        String page = fullPageLink("", ANCHOR_TEXT);
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.SRC_ANCHOR_TYPE, fullPath);
    }
 
    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcAnchorTypeBlankHref() throws Throwable {
        blankHrefTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcAnchorTypeBlankHrefByFocus() throws Throwable {
        blankHrefTestBody(false);
    }

    private void srcAnchorTypeRelativeUrlTestBody(boolean byTouch) throws Throwable {
        String relPath = "/foo.html";
        String fullPath = mWebServer.getResponseUrl(relPath);
        String page = fullPageLink(relPath, ANCHOR_TEXT);
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.SRC_ANCHOR_TYPE, fullPath);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcAnchorTypeRelativeUrl() throws Throwable {
        srcAnchorTypeRelativeUrlTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcAnchorTypeRelativeUrlByFocus() throws Throwable {
        srcAnchorTypeRelativeUrlTestBody(false);
    }

    private void srcEmailTypeTestBody(boolean byTouch) throws Throwable {
        String email = "foo@bar.com";
        String prefix = "mailto:";
        String page = fullPageLink(prefix + email, ANCHOR_TEXT);
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.EMAIL_TYPE, email);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcEmailType() throws Throwable {
        srcEmailTypeTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcEmailTypeByFocus() throws Throwable {
        srcEmailTypeTestBody(false);
    }

    private void srcGeoTypeTestBody(boolean byTouch) throws Throwable {
        String location = "Jilin";
        String prefix = "geo:0,0?q=";
        String page = fullPageLink(prefix + location, ANCHOR_TEXT);
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.GEO_TYPE, location);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcGeoType() throws Throwable {
        srcGeoTypeTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcGeoTypeByFocus() throws Throwable {
        srcGeoTypeTestBody(false);
    }

    private void srcPhoneTypeTestBody(boolean byTouch) throws Throwable {
        String phone_num = "%2B1234567890";
        String expected_phone_num = "+1234567890";
        String prefix = "tel:";
        String page = fullPageLink("tel:" + phone_num, ANCHOR_TEXT);
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.PHONE_TYPE, expected_phone_num);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcPhoneType() throws Throwable {
        srcPhoneTypeTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcPhoneTypeByFocus() throws Throwable {
        srcPhoneTypeTestBody(false);
    }

    private void srcImgeAnchorTypeTestBody(boolean byTouch) throws Throwable {
        String fullImageSrc = "http://foo.bar/nonexistent.jpg";
        String page = CommonResources.makeHtmlPageFrom("", "<a class=\"full_view\" href=\""
                + HREF + "\"onclick=\"return false;\"><img class=\"full_view\" src=\""
                + fullImageSrc + "\"></a>");
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.SRC_IMAGE_ANCHOR_TYPE, fullImageSrc);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcImgeAnchorType() throws Throwable {
        srcImgeAnchorTypeTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcImgeAnchorTypeByFocus() throws Throwable {
        srcImgeAnchorTypeTestBody(false);
    }

    private void srcImgeAnchorTypeRelativeUrlTestBody(boolean byTouch) throws Throwable {
        String relImageSrc = "/nonexistent.jpg";
        String fullImageSrc = mWebServer.getResponseUrl(relImageSrc);
        String relPath = "/foo.html";
        String fullPath = mWebServer.getResponseUrl(relPath);
        String page = CommonResources.makeHtmlPageFrom("", "<a class=\"full_view\" href=\""
                + relPath + "\"onclick=\"return false;\"><img class=\"full_view\" src=\""
                + relImageSrc + "\"></a>");
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.SRC_IMAGE_ANCHOR_TYPE, fullImageSrc);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcImgeAnchorTypeRelativeUrl() throws Throwable {
        srcImgeAnchorTypeRelativeUrlTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testSrcImgeAnchorTypeRelativeUrlByFocus() throws Throwable {
        srcImgeAnchorTypeRelativeUrlTestBody(false);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testImgeType() throws Throwable {
        String relImageSrc = "/"  + CommonResources.TEST_IMAGE_FILENAME;
        String fullImageSrc = mWebServer.getResponseUrl(relImageSrc);
        String page = CommonResources.makeHtmlPageFrom("",
                "<img class=\"full_view\" src=\"" + relImageSrc + "\">");
        setServerResponseAndLoad(page);
        XWalkViewInternalTestTouchUtils.simulateTouchCenterOfView(mXWalkViewInternal);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.IMAGE_TYPE, fullImageSrc);
        // TODO(Chuan): Add requestFocusNodeHref test after implementation
    }

    private void editTextTypeTestBody(boolean byTouch) throws Throwable {
        String page = CommonResources.makeHtmlPageFrom("",
                "<form><input class=\"full_view\" type=\"text\" name=\"test\"></form>");
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.EDIT_TEXT_TYPE, null);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testEditTextType() throws Throwable {
        editTextTypeTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testEditTextTypeByFocus() throws Throwable {
        editTextTypeTestBody(false);
    }

    public void unknownTypeJavascriptSchemeTestBody(boolean byTouch) throws Throwable {
        // Per documentation, javascript urls are special.
        String javascript = "javascript:alert('foo');";
        String page = fullPageLink(javascript, ANCHOR_TEXT);
        setServerResponseAndLoad(page);
        simulateInput(byTouch);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.UNKNOWN_TYPE, null);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testUnknownTypeJavascriptScheme() throws Throwable {
        unknownTypeJavascriptSchemeTestBody(true);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testUnknownTypeJavascriptSchemeByFocus() throws Throwable {
        unknownTypeJavascriptSchemeTestBody(false);
    }

    @SmallTest
    @Feature({"XWalkView", "HitTest"})
    public void testUnknownTypeUnrecognizedNode() throws Throwable {
        // Since UNKNOWN_TYPE is the default, hit test another type first for
        // this test to be valid.
        testSrcAnchorType();

        final String title = "UNKNOWN_TYPE title";

        String page = CommonResources.makeHtmlPageFrom(
                "<title>" + title + "</title>",
                "<div class=\"full_view\">div text</div>");
        setServerResponseAndLoad(page);

        // Wait for the new page to be loaded before trying hit test.
        pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkViewInternal.getTitle().equals(title);
            }
        });
        XWalkViewInternalTestTouchUtils.simulateTouchCenterOfView(mXWalkViewInternal);
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.UNKNOWN_TYPE, null);
    }

    @LargeTest
    @Feature({"XWalkView", "HitTest"})
    public void testUnfocusedNodeAndTouchRace() throws Throwable {
        // Test when the touch and focus paths racing with setting different
        // results.

        String relImageSrc = "/"  + CommonResources.TEST_IMAGE_FILENAME;
        String fullImageSrc = mWebServer.getResponseUrl(relImageSrc);
        String html = CommonResources.makeHtmlPageFrom(
                "<meta name=\"viewport\" content=\"width=device-width,height=device-height\" />"
                + "<style type=\"text/css\">"
                + ".full_width { width:100%; position:absolute; }"
                + "</style>",
                "<form><input class=\"full_width\" style=\"height:25%;\" "
                + "type=\"text\" name=\"test\"></form>"
                + "<img class=\"full_width\" style=\"height:50%;top:25%;\" "
                + "src=\"" + relImageSrc + "\">");
        setServerResponseAndLoad(html);

        // Focus on input element and check the hit test results.
        simulateTabDownUpOnUiThread();
        pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.EDIT_TEXT_TYPE, null);

        // Touch image. Now the focus based hit test path will try to null out
        // the results and the touch based path will update with the result of
        // the image.
        XWalkViewInternalTestTouchUtils.simulateTouchCenterOfView(mXWalkViewInternal);

        // Make sure the result of image sticks.
        for (int i = 0; i < 2; ++i) {
            Thread.sleep(500);
            pollForHitTestDataOnUiThread(XWalkHitTestResultInternal.type.IMAGE_TYPE, fullImageSrc);
        }
    }
}
