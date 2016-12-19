// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test.util;

import android.util.Pair;
import java.util.ArrayList;
import java.util.List;

// Auxiliary class providing common HTML and base64 resources using for testing.
public class CommonResources {

    // Content-type headers used for HTML code.
    public static List<Pair<String, String>> getTextHtmlHeaders(boolean disableCache) {
        return getContentTypeAndCacheHeaders("text/html", disableCache);
    }

    // Content-type headers used for javascript code.
    public static List<Pair<String, String>> getTextJavascriptHeaders(boolean disableCache) {
        return getContentTypeAndCacheHeaders("text/javascript", disableCache);
    }

    // Content-type headers used for png images.
    public static List<Pair<String, String>> getImagePngHeaders(boolean disableCache) {
        return getContentTypeAndCacheHeaders("image/png", disableCache);
    }

    public static List<Pair<String, String>> getContentTypeAndCacheHeaders(
            String contentType, boolean disableCache) {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", contentType));
        if (disableCache) headers.add(Pair.create("Cache-Control", "no-store"));
        return headers;
    }

    // Returns the HTML code used to verify if an image has been successfully loaded.
    public static String getOnImageLoadedHtml(String imageSrc) {
        return "<html>\n" +
               "  <head>\n" +
               "    <script>\n" +
               "      function updateTitle() {\n" +
               "        document.title=document.getElementById('img').naturalHeight\n" +
               "      }\n" +
               "    </script>\n" +
               "  </head>\n" +
               "  <body onload='updateTitle();'>\n" +
               "    <img id='img' onload='updateTitle();' src='" + imageSrc + "'>\n" +
               "  </body>\n" +
               "</html>\n";
    }

    // Default name for the favicon image.
    public static final String FAVICON_FILENAME = "favicon.png";

    // Default name for the test image.
    public static final String TEST_IMAGE_FILENAME = "testimage.png";

    // HTML code of a static simple page with a favicon.
    public static final String FAVICON_STATIC_HTML =
        "<html><head><link rel=\"icon\" type=\"image/png\" href=\"" + FAVICON_FILENAME + "\">" +
        "</head><body>Favicon example</body></html>";

    // Base64 data of a favicon image resource.
    public static final String FAVICON_DATA_BASE64 =
        "iVBORw0KGgoAAAANSUhEUgAAABAAAAAFCAYAAABM6GxJAAAABHNCSVQICAgIfAhkiAAAASJJREFU" +
        "GJU9yDtLQnEYwOHfOZ40L3gZDJKgJCKaamvpGzS09wUaormh7xA0S5C0ZDTkZJsNUltkkpAUZkIX" +
        "L3g9FzzH/9vm9vAgoqRUGUu20JHTXFfafUdERJSIKJnOPFUTERHpqIYclY5nb2QKFumky95OlO+W" +
        "TSgATqOO5k3xr6ZxelXmDFDhdaqfLkPRWQglULaN/V5DPzl3iIb9xCI+Eskog/wdyhowLlb4vThE" +
        "giF8zRsurx55beg8lMfMezZW9hqz20M/Owhwe2/yUrPI5Ds8//mRehN7JYWxvIX6eWJkbLK9laL8" +
        "ZrKxFETzxTBNB5SOJjKV/mhCq+uSjGvE4hHc4QA9YGAEwnhWF1ePkCtOWFv0+PiasL8bR3QDr93h" +
        "HyFup9LWUksHAAAAAElFTkSuQmCC";

    // Default name for an example 'about' HTML page.
    public static final String ABOUT_FILENAME = "about.html";

    // Title used in the 'about' example.
    public static final String ABOUT_TITLE = "About the Google";

    // HTML code of an 'about' example.
    public static final String ABOUT_HTML =
        "<html>" +
        "  <head>" +
        "    <title>" + ABOUT_TITLE + "</title>" +
        "  </head>" +
        "  <body>" +
        "    This is the Google!" +
        "  </body>" +
        "</html>";

    public static String makeHtmlPageFrom(String headers, String body) {
        return "<html>" +
                 "<head>" +
                     "<style type=\"text/css\">" +
                         // Make the image take up all of the page so that we don't have to do
                         // any fancy hit target calculations when synthesizing the touch event
                         // to click it.
                         "img.big { width:100%; height:100%; background-color:blue; }" +
                         ".full_view { height:100%; width:100%; position:absolute; }" +
                     "</style>" +
                     headers +
                 "</head>" +
                 "<body>" +
                     body +
                 "</body>" +
             "</html>";
    }
}
