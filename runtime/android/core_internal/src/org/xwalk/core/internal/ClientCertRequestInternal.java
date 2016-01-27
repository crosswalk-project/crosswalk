// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.security.Principal;
import java.security.PrivateKey;
import java.util.List;

/**
 * ClientCertRequest: The user receives an instance of this class as a parameter of 
 * {@link XWalkResourceClient#onReceivedClientCertRequest}.
 * The request includes the parameters to choose the client certificate, such as the host name 
 * and the port number requesting the cert.
 *
 * The user should call one of the class methods to indicate how to deal with the client 
 * certificate request. All methods should be called on UI thread.
 */
@XWalkAPI(instance = ClientCertRequestHandlerInternal.class)
public interface ClientCertRequestInternal {

    /**
     * Cancel this request. Remember the user's choice and use it for future requests.
     * @since 6.0
     */
    @XWalkAPI
    public void cancel();

    /**
     * Proceed with the specified private key and client certificate chain.
     * Remember the user's positive choice and use it for future requests.
     * @param privateKey the private Key
     * @param chain the certificate chain
     * @since 6.0
     */
    @XWalkAPI
    public void proceed(final PrivateKey privateKey, final List<X509Certificate> chain);

    /**
     * Ignore the request for now. Do not remember user's choice.
     * @since 6.0
     */
    @XWalkAPI
    public void ignore();

    /**
     * Returns the host name of the server requesting the certificate.
     * @return host name of the server requesting the certificate.
     * @since 6.0
     */
    @XWalkAPI
    public String getHost();

    /**
     * Returns the port number of the server requesting the certificate.
     * @return port number of the server requesting the certificate.
     * @since 6.0
     */
    @XWalkAPI
    public int getPort();

    /**
     * Gets the acceptable types of asymmetric keys (can be null).
     * @return the acceptable types of asymmetric keys such as "EC" or "RSA",
     *         or a null array.
     * @since 6.0
     */
    @XWalkAPI
    public String[] getKeyTypes();

    /**
     * Gets the acceptable certificate issuers for the certificate matching the
     * private key (can be null).
     * @return the acceptable certificate issuers for the certificate matching
     *         the private key, or null.
     * @since 6.0
     */
    @XWalkAPI
    public Principal[] getPrincipals();
}
