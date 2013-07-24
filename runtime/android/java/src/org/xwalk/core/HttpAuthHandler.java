/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.xwalk.core;

import android.os.Handler;

/**
 * Represents a request for HTTP authentication. Instances of this class are
 * created by the XWalkView and passed to
 * {@link XWalkClient#onReceivedHttpAuthRequest}. The host application must
 * call either {@link #proceed} or {@link #cancel} to set the XWalkView's
 * response to the request.
 */
public class HttpAuthHandler extends Handler {

    /**
     * @hide Only for use by WebViewProvider implementations.
     */
    public HttpAuthHandler() {
    }

    /**
     * Gets whether the credentials stored for the current host (i.e. the host
     * for which {@link XWalkClient#onReceivedHttpAuthRequest} was called)
     * are suitable for use. Credentials are not suitable if they have
     * previously been rejected by the server for the current request.
     *
     * @return whether the credentials are suitable for use
     * @see XWalkView#getHttpAuthUsernamePassword
     */
    public boolean useHttpAuthUsernamePassword() {
        return false;
    }

    /**
     * Instructs the XWalkView to cancel the authentication request.
     */
    public void cancel() {
    }

    /**
     * Instructs the XWalkView to proceed with the authentication with the given
     * credentials. Credentials for use with this method can be retrieved from
     * the XWalkView's store using {@link XWalkView#getHttpAuthUsernamePassword}.
     */
    public void proceed(String username, String password) {
    }

    /**
     * Gets whether the prompt dialog should be suppressed.
     *
     * @return whether the prompt dialog should be suppressed
     * @hide
     */
    public boolean suppressDialog() {
        return false;
    }
}
