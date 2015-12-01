// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.util.Log;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

import java.io.InputStream;
import java.io.StringBufferInputStream;
import java.util.Map;

/**
 * Encapsulates a resource response. Applications can return an instance of this
 * class from {@link XWalkResourceClient#shouldInterceptRequest} to provide a custom
 * response when the XWalkView requests a particular resource.
 */
@JNINamespace("xwalk")
@XWalkAPI(createInternally = true)
public class XWalkWebResourceResponseInternal{
    private String mMimeType;
    private String mEncoding;
    private InputStream mData;
    private int mStatusCode;
    private String mReasonPhrase;
    private Map<String, String> mResponseHeaders;
    private String[] mResponseHeaderNames;
    private String[] mResponseHeaderValues;

    // Never use this constructor.
    // It is only used in XWalkWebResourceResponseBridge.
    XWalkWebResourceResponseInternal() {
        mMimeType = null;
        mEncoding = null;
        mData = null;
    }

    XWalkWebResourceResponseInternal(String mimeType, String encoding, InputStream data) {
        mMimeType = mimeType;
        mEncoding = encoding;
        setData(data);
    }

    XWalkWebResourceResponseInternal(String mimeType, String encoding, InputStream data,
            int statusCode, String reasonPhrase, Map<String, String> responseHeaders) {
        this(mimeType, encoding, data);

        mStatusCode = statusCode;
        mReasonPhrase = reasonPhrase;
        mResponseHeaders = responseHeaders;
    }

    private void fillInResponseHeaderNamesAndValuesIfNeeded() {
        if (mResponseHeaders == null || mResponseHeaderNames != null) return;
        mResponseHeaderNames = new String[mResponseHeaders.size()];
        mResponseHeaderValues = new String[mResponseHeaders.size()];
        int i = 0;
        for (Map.Entry<String, String> entry : mResponseHeaders.entrySet()) {
            mResponseHeaderNames[i] = entry.getKey();
            mResponseHeaderValues[i] = entry.getValue();
            i++;
        }
    }

    /**
     * Sets the resource response's MIME type, for example &quot;text/html&quot;.
     *
     * @param mimeType The resource response's MIME type
     * @since 6.0
     */
    @XWalkAPI
    public void setMimeType(String mimeType) {
        mMimeType = mimeType;
    }

    /**
     * Gets the resource response's MIME type.
     *
     * @return The resource response's MIME type
     * @since 6.0
     */
    @XWalkAPI
    public String getMimeType() {
        return mMimeType;
    }

    @CalledByNative
    public String getMimeTypeNative() {
        return mMimeType;
    }

    /**
     * Sets the resource response's encoding, for example &quot;UTF-8&quot;. This is used
     * to decode the data from the input stream.
     *
     * @param encoding The resource response's encoding
     * @since 6.0
     */
    @XWalkAPI
    public void setEncoding(String encoding) {
        mEncoding = encoding;
    }

    /**
     * Gets the resource response's encoding.
     *
     * @return The resource response's encoding
     * @since 6.0
     */
    @XWalkAPI
    public String getEncoding() {
        return mEncoding;
    }

    @CalledByNative
    public String getEncodingNative() {
        return mEncoding;
    }

    /**
     * Sets the input stream that provides the resource response's data. Callers
     * must implement {@link InputStream#read(byte[]) InputStream.read(byte[])}.
     *
     * @param data the input stream that provides the resource response's data. Must not be a
     *             StringBufferInputStream.
     * @since 6.0
     */
    @XWalkAPI
    public void setData(InputStream data) {
        // If data is (or is a subclass of) StringBufferInputStream
        if (data != null && StringBufferInputStream.class.isAssignableFrom(data.getClass())) {
            throw new IllegalArgumentException("StringBufferInputStream is deprecated and must " +
                "not be passed to a XWalkWebResourceResponse");
        }
        mData = data;
    }

    /**
     * Gets the input stream that provides the resource response's data.
     *
     * @return The input stream that provides the resource response's data
     * @since 6.0
     */
    @XWalkAPI
    public InputStream getData() {
        return mData;
    }

    @CalledByNative
    public InputStream getDataNative() {
        return mData;
    }

    /**
     * Sets the resource response's status code and reason phrase.
     *
     * @param statusCode the status code needs to be in the ranges [100, 299], [400, 599].
     *                   Causing a redirect by specifying a 3xx code is not supported.
     * @param reasonPhrase the phrase describing the status code, for example "OK". Must be non-null
     *                     and not empty.
     * @since 6.0
     */
    @XWalkAPI
    public void setStatusCodeAndReasonPhrase(int statusCode, String reasonPhrase) {
        if (statusCode < 100)
            throw new IllegalArgumentException("statusCode can't be less than 100.");
        if (statusCode > 599)
            throw new IllegalArgumentException("statusCode can't be greater than 599.");
        if (statusCode > 299 && statusCode < 400)
            throw new IllegalArgumentException("statusCode can't be in the [300, 399] range.");
        if (reasonPhrase == null)
            throw new IllegalArgumentException("reasonPhrase can't be null.");
        if (reasonPhrase.trim().isEmpty())
            throw new IllegalArgumentException("reasonPhrase can't be empty.");
        for (int i = 0; i < reasonPhrase.length(); i++) {
            int c = reasonPhrase.charAt(i);
            if (c > 0x7F) {
                throw new IllegalArgumentException(
                        "reasonPhrase can't contain non-ASCII characters.");
            }
        }
        mStatusCode = statusCode;
        mReasonPhrase = reasonPhrase;
    }

    /**
     * Gets the resource response's status code.
     *
     * @return The resource response's status code.
     * @since 6.0
     */
    @XWalkAPI
    public int getStatusCode() {
        return mStatusCode;
    }

    @CalledByNative
    public int getStatusCodeNative() {
        return mStatusCode;
    }

    /**
     * Gets the description of the resource response's status code.
     *
     * @return The description of the resource response's status code.
     * @since 6.0
     */
    @XWalkAPI
    public String getReasonPhrase() {
        return mReasonPhrase;
    }

    @CalledByNative
    public String getReasonPhraseNative() {
        return mReasonPhrase;
    }

    /**
     * Sets the headers for the resource response.
     *
     * @param headers Mapping of header name to header value.
     * @since 6.0
     */
    @XWalkAPI
    public void setResponseHeaders(Map<String, String> headers) {
        mResponseHeaders = headers;
    }

    /**
     * Gets the headers for the resource response.
     *
     * @return The headers for the resource response.
     * @since 6.0
     */
    @XWalkAPI
    public Map<String, String> getResponseHeaders() {
        return mResponseHeaders;
    }

    @CalledByNative
    private String[] getResponseHeaderNames() {
        fillInResponseHeaderNamesAndValuesIfNeeded();
        return mResponseHeaderNames;
    }

    @CalledByNative
    private String[] getResponseHeaderValues() {
        fillInResponseHeaderNamesAndValuesIfNeeded();
        return mResponseHeaderValues;
    }
}
