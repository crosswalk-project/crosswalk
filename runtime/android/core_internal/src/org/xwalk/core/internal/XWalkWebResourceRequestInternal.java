/*
 * Copyright (C) 2014 The Android Open Source Project
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

package org.xwalk.core.internal;

import android.net.Uri;

import java.util.Map;

/**
 * Encompasses parameters to the {@link XWalkResourceClient#shouldInterceptLoadRequest} method.
 */
@XWalkAPI(instance = XWalkWebResourceRequestHandlerInternal.class)
public interface XWalkWebResourceRequestInternal {
    /**
     * Gets the URL for which the resource request was made.
     *
     * @return the URL for which the resource request was made.
     * @since 6.0
     */
    @XWalkAPI
    public Uri getUrl();

    /**
     * Gets whether the request was made for the main frame.
     *
     * @return whether the request was made for the main frame. Will be false for iframes,
     *         for example.
     * @since 6.0
     */
    @XWalkAPI
    public boolean isForMainFrame();

    /**
     * Gets whether a gesture (such as a click) was associated with the request.
     * For security reasons in certain situations this method may return false even though the
     * sequence of events which caused the request to be created was initiated by a user gesture.
     *
     * @return whether a gesture was associated with the request.
     * @since 6.0
     */
    @XWalkAPI
    public boolean hasGesture();

    /**
     * Gets the method associated with the request, for example "GET".
     *
     * @return the method associated with the request.
     * @since 6.0
     */
    @XWalkAPI
    public String getMethod();

    /**
     * Gets the headers associated with the request. These are represented as a mapping of header
     * name to header value.
     *
     * @return the headers associated with the request.
     * @since 6.0
     */
    @XWalkAPI
    public Map<String, String> getRequestHeaders();
}
