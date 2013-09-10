// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.app.Activity;
import android.content.Context;

/**
 * This is a public class to provide context for extensions.
 * It'll be shared by all extensions.
 */
public abstract class XWalkExtensionContext {
    /**
     * Register the current extension into system and return the unique ID
     * from extension system.
     * @return the registered extension ID.
     */
    public abstract Object registerExtension(XWalkExtension extension);
    
    /**
     * Unregister the given extension from extension system.
     * @param extension the extension which needs to be unregistered.
     */
    public abstract void unregisterExtension(XWalkExtension extension);

    /**
     * Post message to JavaScript via internal mechanism.
     * @param extension the extension which needs to post message to JavaScript.
     * @param message the message to be passed.
     */
    public abstract void postMessage(XWalkExtension extension, String message);

    /**
     * Get current Android Context.
     * @return the current Android Context.
     */
    public abstract Context getContext();

    /**
     * Get the current Android Activity.
     * @return the current Android Activity.
     */
    public abstract Activity getActivity();
}
