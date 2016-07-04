// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.net.http.SslCertificate;
import android.net.http.SslError;
import android.util.Log;

import org.chromium.net.NetError;
import org.chromium.net.X509Util;

import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

class SslUtil {
    private static final String TAG = "SslUtil";

    /**
     * Creates an SslError object from a chromium net error code.
     */
    public static SslError sslErrorFromNetErrorCode(int error, SslCertificate cert, String url) {
        assert (error >= NetError.ERR_CERT_END && error <= NetError.ERR_CERT_COMMON_NAME_INVALID);

        return new SslError(SslError.SSL_INVALID, cert, url);
    }

    public static SslCertificate getCertificateFromDerBytes(byte[] derBytes) {
        if (derBytes == null) {
            return null;
        }

        try {
            X509Certificate x509Certificate =
                    X509Util.createCertificateFromBytes(derBytes);
            return new SslCertificate(x509Certificate);
        } catch (CertificateException e) {
            // A SSL related exception must have occured.  This shouldn't happen.
            Log.w(TAG, "Could not read certificate: " + e);
        } catch (KeyStoreException e) {
            // A SSL related exception must have occured.  This shouldn't happen.
            Log.w(TAG, "Could not read certificate: " + e);
        } catch (NoSuchAlgorithmException e) {
            // A SSL related exception must have occured.  This shouldn't happen.
            Log.w(TAG, "Could not read certificate: " + e);
        }
        return null;
    }

    public static boolean shouldDenyRequest(int error) {
        assert (error >= NetError.ERR_CERT_END && error <= NetError.ERR_CERT_COMMON_NAME_INVALID);
        // Why deny the request for these errors, please refer to the comment in
        // https://github.com/crosswalk-project/chromium-crosswalk/blob/master/content/browser/ssl/ssl_policy.cc#L61
        // and https://github.com/crosswalk-project/chromium-crosswalk/blob/master/content/browser/ssl/ssl_policy.cc#L89
        // In Chrome, please refer to https://github.com/crosswalk-project/chromium-crosswalk/blob/master/chrome/browser/chrome_content_browser_client.cc#L2019,
        // the errors were passed to AllowCertificateError(), then display an SSL blocking page.
        switch(error) {
            case NetError.ERR_CERT_COMMON_NAME_INVALID:
            case NetError.ERR_CERT_DATE_INVALID:
            case NetError.ERR_CERT_AUTHORITY_INVALID:
            case NetError.ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
            case NetError.ERR_CERT_WEAK_KEY:
            case NetError.ERR_CERT_NAME_CONSTRAINT_VIOLATION:
            case NetError.ERR_CERT_VALIDITY_TOO_LONG:
            case NetError.ERR_CERT_CONTAINS_ERRORS:
            case NetError.ERR_CERT_REVOKED:
            case NetError.ERR_CERT_INVALID:
            case NetError.ERR_SSL_WEAK_SERVER_EPHEMERAL_DH_KEY:
            case NetError.ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN:
                return true;
            default:
                break;
        }
        return false;
    }
}
