// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.util.Log;

import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.security.Principal;
import java.security.PrivateKey;
import java.util.List;

import org.chromium.base.ThreadUtils;
import org.chromium.net.AndroidPrivateKey;
import org.chromium.net.DefaultAndroidKeyStore;

@XWalkAPI(impl = ClientCertRequestInternal.class, createInternally = true)
public class ClientCertRequestHandlerInternal implements ClientCertRequestInternal {
    private static final String TAG = "ClientCertRequestHandlerInternal";
    private XWalkContentsClientBridge mContentsClient;
    private int mId;
    private String mHost;
    private int mPort;
    private boolean mIsCalled;

    ClientCertRequestHandlerInternal(XWalkContentsClientBridge contentsClient, int id, String host,
            int port) {
        mId = id;
        mHost = host;
        mPort = port;
        mContentsClient = contentsClient;
    }

    // Never use this constructor.
    // It is only used in ClientCertRequestHandlerBridge.
    ClientCertRequestHandlerInternal() {
        mId = -1;
        mHost = "";
        mPort = -1;
        mContentsClient = null;
    }

    @XWalkAPI
    public void proceed(final PrivateKey privateKey, final List<X509Certificate> chain) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                X509Certificate[] chains = null;
                if (chain != null) {
                    chains = chain.toArray(new X509Certificate[chain.size()]);
                }
                proceedOnUiThread(privateKey, chains);
            }
        });
    }

    @XWalkAPI
    public void ignore() {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ignoreOnUiThread();
            }
        });
    }

    @XWalkAPI
    public void cancel() {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                cancelOnUiThread();
            }
        });
    }

    @XWalkAPI
    public String getHost() {
        return mHost;
    }

    @XWalkAPI
    public int getPort() {
        return mPort;
    }

    private void proceedOnUiThread(PrivateKey privateKey, X509Certificate[] chain) {
        checkIfCalled();
        AndroidPrivateKey key = mContentsClient.mLocalKeyStore.createKey(privateKey);

        if ((key == null) || (chain == null) || (chain.length == 0)) {
            Log.w(TAG, "Empty client certificate chain?");
            provideResponse(null, null);

            return;
        }

        // Encode the certificate chain.
        byte[][] encodedChain = new byte[chain.length][];

        try {
            for (int i = 0; i < chain.length; ++i) {
                encodedChain[i] = chain[i].getEncoded();
            }
        } catch (CertificateEncodingException e) {
            Log.w(TAG, "Could not retrieve encoded certificate chain: " + e);
            provideResponse(null, null);

            return;
        }

        mContentsClient.mLookupTable.allow(mHost, mPort, key, encodedChain);
        provideResponse(key, encodedChain);
    }

    private void ignoreOnUiThread() {
        checkIfCalled();
        provideResponse(null, null);
    }

    private void cancelOnUiThread() {
        checkIfCalled();
        mContentsClient.mLookupTable.deny(mHost, mPort);
        provideResponse(null, null);
    }

    private void checkIfCalled() {
        if (mIsCalled) {
            throw new IllegalStateException("The callback was already called.");
        }

        mIsCalled = true;
    }

    private void provideResponse(AndroidPrivateKey androidKey, byte[][] certChain) {
        mContentsClient.provideClientCertificateResponse(mId, certChain, androidKey);
    }
}
