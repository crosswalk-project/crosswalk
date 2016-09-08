// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.util.Log;

import java.net.MalformedURLException;
import java.net.URL;

import org.chromium.base.annotations.JNINamespace;

/**
 * XWalkCookieManager manages cookies according to RFC2109 spec.
 * Methods in this class are thread safe.
 * @hide
 */
@JNINamespace("xwalk")
@XWalkAPI(createExternally = true)
public class XWalkCookieManagerInternal {
    private static final String TAG = "XWalkCookieManager";

    /**
     * Control whether cookie is enabled or disabled
     * @param accept TRUE if accept cookie
     * @since 5.0
     */
    @XWalkAPI
    public void setAcceptCookie(boolean accept) {
        nativeSetAcceptCookie(accept);
    }

    /**
     * Return whether cookie is enabled
     * @return TRUE if accept cookie
     * @since 5.0
     */
    @XWalkAPI
    public boolean acceptCookie() {
        return nativeAcceptCookie();
    }

    /**
     * Set cookie for a given url. The old cookie with same host/path/name will
     * be removed. The new cookie will be added if it is not expired or it does
     * not have expiration which implies it is session cookie.
     * @param url The url which cookie is set for
     * @param value The value for set-cookie: in http response header
     * @since 5.0
     */
    @XWalkAPI
    public void setCookie(final String url, final String value) {
        try {
            nativeSetCookie(new URL(url).toString(), value);
        } catch (MalformedURLException e) {
            Log.e(TAG, "Not setting cookie due to invalid URL", e);
        }
    }

    /**
     * Get cookie(s) for a given url so that it can be set to "cookie:" in http
     * request header.
     * @param url The url needs cookie
     * @return The cookies in the format of NAME=VALUE [; NAME=VALUE]
     * @since 5.0
     */
    @XWalkAPI
    public String getCookie(final String url) {
        try {
            String cookie = nativeGetCookie(new URL(url).toString());
            // Return null if the string is empty to match legacy behavior
            return cookie == null || cookie.trim().isEmpty() ? null : cookie;
        } catch (MalformedURLException e) {
            Log.e(TAG, "Unable to get cookies due to invalid URL", e);
            return null;
        }
    }

    /**
     * Remove all session cookies, which are cookies without expiration date
     * @since 5.0
     */
    @XWalkAPI
    public void removeSessionCookie() {
        nativeRemoveSessionCookie();
    }

    /**
     * Remove all cookies
     * @since 5.0
     */
    @XWalkAPI
    public void removeAllCookie() {
        nativeRemoveAllCookie();
    }

    /**
     * Get whether there are stored cookies.
     * @return true if there are stored cookies
     * @since 5.0
     */
    @XWalkAPI
    public boolean hasCookies() {
        return nativeHasCookies();
    }

    /**
     * Remove all expired cookies
     * @since 5.0
     */
    @XWalkAPI
    public void removeExpiredCookie() {
        nativeRemoveExpiredCookie();
    }

    /**
     * Flush cookies store
     * @since 5.0
     */
    @XWalkAPI
    public void flushCookieStore() {
        nativeFlushCookieStore();
    }

    /**
     * Get whether cookies are accepted for file scheme URLs.
     * @return true if cookies are accepted for file scheme URLs
     * @since 5.0
     */
    @XWalkAPI
    public boolean allowFileSchemeCookies() {
        return nativeAllowFileSchemeCookies();
    }

    /**
     * Sets whether cookies are accepted for file scheme URLs.
     *
     * Use of cookies with file scheme URLs is potentially insecure. Do not
     * use this feature unless you can be sure that no unintentional sharing
     * of cookie data can take place.
     * <p>
     * Note that calls to this method will have no effect if made after a
     * CookieManager instance has done any real work.
     * @param accept Whether accept cookies for file scheme URLs
     * @since 5.0
     */
    @XWalkAPI
    public void setAcceptFileSchemeCookies(boolean accept) {
        nativeSetAcceptFileSchemeCookies(accept);
    }

    private native void nativeSetAcceptCookie(boolean accept);
    private native boolean nativeAcceptCookie();

    private native void nativeSetCookie(String url, String value);
    private native String nativeGetCookie(String url);

    private native void nativeRemoveSessionCookie();
    private native void nativeRemoveAllCookie();
    private native void nativeRemoveExpiredCookie();
    private native void nativeFlushCookieStore();

    private native boolean nativeHasCookies();

    private native boolean nativeAllowFileSchemeCookies();
    private native void nativeSetAcceptFileSchemeCookies(boolean accept);
}
