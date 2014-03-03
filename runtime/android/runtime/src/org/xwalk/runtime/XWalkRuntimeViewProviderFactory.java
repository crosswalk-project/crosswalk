// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.app.Activity;
import android.content.Context;

/**
 * The factory to create a XWalkRuntimeViewProvider instance according to
 * current setting.
 */
final class XWalkRuntimeViewProviderFactory {
    static public XWalkRuntimeViewProvider getProvider(Context context, Activity activity) {
        // TODO(yongsheng): Do checkings here to decide which provider should
        // be used. The default is to use runtime core provider.
        return new XWalkCoreProviderImpl(context, activity);
    }
}
