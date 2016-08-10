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
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.io.BufferedWriter;
import java.io.File;
import java.util.Map;

@XWalkAPI(createInternally = true)
public class XWalkHitTestResultInternal {
    @XWalkAPI
    public enum type {
        UNKNOWN_TYPE,
        ANCHOR_TYPE,
        PHONE_TYPE,
        GEO_TYPE,
        EMAIL_TYPE,
        IMAGE_TYPE,
        IMAGE_ANCHOR_TYPE,
        SRC_ANCHOR_TYPE,
        SRC_IMAGE_ANCHOR_TYPE,
        EDIT_TEXT_TYPE
    }

    private int mType;
    private String mExtra;

    /**
     * @hide Only for use by XWalkViewProvider implementations
     */
    public XWalkHitTestResultInternal() {
        mType = 0;
    }

    /**
     * @hide Only for use by XWalkViewProvider implementations
     */
    public void setType(int type) {
        mType = type;
    }

    /**
     * @hide Only for use by XWalkViewProvider implementations
     */
    public void setExtra(String extra) {
        mExtra = extra;
    }

    /**
     * Gets the type of the hit test result. See the XXX_TYPE constants
     * defined in this class.
     *
     * @return the type of the hit test result
     * @since 7.0
     */
    @XWalkAPI
    public type getType() {
        type a;
        switch (mType){
            case 0:
              a = type.UNKNOWN_TYPE;
            break;
            case 1:
              a = type.ANCHOR_TYPE;
            break;
            case 2:
              a = type.PHONE_TYPE;
            break;
            case 3:
              a = type.GEO_TYPE;
            break;
            case 4:
              a = type.EMAIL_TYPE;
            break;
            case 5:
              a = type.IMAGE_TYPE;
            break;
            case 6:
              a = type.IMAGE_ANCHOR_TYPE;
            break;
            case 7:
              a = type.SRC_ANCHOR_TYPE;
            break;
            case 8:
              a = type.SRC_IMAGE_ANCHOR_TYPE;
            break;
            case 9:
              a = type.EDIT_TEXT_TYPE;
            break;
            default:
              a = type.UNKNOWN_TYPE;
            break;
        }
        return a;
    }

    /**
     * Gets additional type-dependant information about the result. See
     * {@link WebView#getHitTestResult()} for details. May either be null
     * or contain extra information about this result.
     *
     * @return additional type-dependant information about the result
     * @since 7.0
     */
    @XWalkAPI
    public String getExtra() {
        return mExtra;
    }
}
