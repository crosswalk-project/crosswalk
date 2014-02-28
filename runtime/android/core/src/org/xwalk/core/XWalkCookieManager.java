// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.base.JNINamespace;

/**
 * XWalkCookieManager manages cookies according to RFC2109 spec.
 *
 * Methods in this class are thread safe.
 */
@JNINamespace("xwalk")
public final class XWalkCookieManager {
    /**
     * Control whether cookie is enabled or disabled
     * @param accept TRUE if accept cookie
     */
    public void setAcceptCookie(boolean accept) {
        nativeSetAcceptCookie(accept);
    }

    /**
     * Return whether cookie is enabled
     * @return TRUE if accept cookie
     */
    public boolean acceptCookie() {
        return nativeAcceptCookie();
    }

    /**
     * Set cookie for a given url. The old cookie with same host/path/name will
     * be removed. The new cookie will be added if it is not expired or it does
     * not have expiration which implies it is session cookie.
     * @param url The url which cookie is set for
     * @param value The value for set-cookie: in http response header
     */
    public void setCookie(final String url, final String value) {
        nativeSetCookie(url, value);
    }

    /**
     * Get cookie(s) for a given url so that it can be set to "cookie:" in http
     * request header.
     * @param url The url needs cookie
     * @return The cookies in the format of NAME=VALUE [; NAME=VALUE]
     */
    public String getCookie(final String url) {
        String cookie = nativeGetCookie(url.toString());
        // Return null if the string is empty to match legacy behavior
        return cookie == null || cookie.trim().isEmpty() ? null : cookie;
    }

    /**
     * Remove all session cookies, which are cookies without expiration date
     */
    public void removeSessionCookie() {
        nativeRemoveSessionCookie();
    }

    /**
     * Remove all cookies
     */
    public void removeAllCookie() {
        nativeRemoveAllCookie();
    }

    /**
     *  Return true if there are stored cookies.
     */
    public boolean hasCookies() {
        return nativeHasCookies();
    }

    /**
     * Remove all expired cookies
     */
    public void removeExpiredCookie() {
        nativeRemoveExpiredCookie();
    }

    public void flushCookieStore() {
        nativeFlushCookieStore();
    }

    /**
     * Whether cookies are accepted for file scheme URLs.
     */
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
     * WebView or CookieManager instance has been created.
     */
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
