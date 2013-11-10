// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.app.Activity;
import android.content.Context;

/**
 * This is a public class to provide context for extensions.
 * It'll be shared by all external extensions.
 */
public class XWalkExtensionContextWrapper extends XWalkExtensionContext {
    private XWalkExtensionContext mOriginContext;

    public XWalkExtensionContextWrapper(XWalkExtensionContext context) {
        mOriginContext = context;
    }

    public Object registerExtension(XWalkExtension extension) {
        return mOriginContext.registerExtension(extension);
    }

    public void unregisterExtension(XWalkExtension extension) {
        mOriginContext.unregisterExtension(extension);
    }

    public void postMessage(XWalkExtension extension, int instanceID, String message) {
        mOriginContext.postMessage(extension, instanceID, message);
    }

    public void broadcastMessage(XWalkExtension extension, String message) {
        mOriginContext.broadcastMessage(extension, message);
    }

    public void destroyExtension(XWalkExtension extension) {
        mOriginContext.destroyExtension(extension);
    }

    public Context getContext() {
        // This is very tricky because for external extensions, we should
        // use Activity which contains the context for runtime client side.
        // mOriginContext.getContext() returns the context of library package,
        // e.g., the package context of runtime side.
        return mOriginContext.getActivity();
    }

    public Activity getActivity() {
        return mOriginContext.getActivity();
    }
}
