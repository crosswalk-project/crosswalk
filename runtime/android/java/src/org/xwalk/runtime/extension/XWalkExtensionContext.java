// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.app.Activity;
import android.content.Context;

/**
 * Interface for extension context
 *
 * It is responsible for maintaining all xwalk extensions and providing a way to
 * post message to JavaScript side for each xwalk extension.
 */
public interface XWalkExtensionContext {
    /**
     * Register an xwalk extension into context.
     */
    public void registerExtension(XWalkExtension extension);

    /**
     * Unregister an xwalk extension with the given unique name from context.
     */
    public void unregisterExtension(String name);

    /**
     * Post a message to the given extension instance.
     *
     * @param extension The xwalk extension
     * @param instanceId The unique id to identify the extension instance as the
     *                   message destination.
     * @param message The message content to be posted.
     */
    public void postMessage(XWalkExtension extension, int instanceId, String message);

    /**
     * Broadcast a message to all extension instances.
     *
     * @param extension The xwalk extension
     * @param message The message content to be broadcasted.
     */
    public void broadcastMessage(XWalkExtension extension, String message);

    /**
     * Get current Android Context.
     * @return the current Android Context.
     */
    public Context getContext();

    /**
     * Get the current Android Activity.
     * @return the current Android Activity.
     */
    public Activity getActivity();
}
