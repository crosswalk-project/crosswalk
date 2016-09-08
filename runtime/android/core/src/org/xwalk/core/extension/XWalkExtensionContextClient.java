// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

/**
 * Interface for extension context
 *
 * It is responsible for maintaining all xwalk extensions and providing a way to
 * post message to JavaScript side for each xwalk extension.
 */
public interface XWalkExtensionContextClient {
    /**
     * Register an xwalk extension into context.
     *
     * @param extension the external extension to be registered
     */
    public void registerExtension(XWalkExternalExtension extension);

    /**
     * Unregister an xwalk extension with the given unique name from context.
     *
     * @param name the name of the external extension to be unregistered
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
    public void postMessage(XWalkExternalExtension extension, int instanceId, String message);

    /**
     * Post a binary message to the given extension instance.
     * JavaScript receives the binary message in an ArrayBuffer.
     *
     * @param extension The xwalk extension
     * @param instanceId The unique id to identify the extension instance as the
     *                   message destination.
     * @param message The binary message content to be posted.
     */
    public void postBinaryMessage(XWalkExternalExtension extension, int instanceId, byte[] message);

    /**
     * Broadcast a message to all extension instances.
     *
     * @param extension The xwalk extension
     * @param message The message content to be broadcasted.
     */
    public void broadcastMessage(XWalkExternalExtension extension, String message);

    /**
     * Get current Android Context.
     * @return the current Android Context.
     */
    public Context getContext();

    /**
     * Get the current Android Activity.
     * @return the current Android Activity.
     * @deprecated This method is no longer supported
     */
    public Activity getActivity();

    /**
     * Start another activity to get some data back.
     * External extensions should call this function to ensure
     * they can get their onActivityResultCallback() be called correctly.
     * @param intent the intent to start.
     * @param requestCode the request code.
     * @param options additional options for how the Activity should be started.
     * @deprecated This method is no longer supported
     */
    public void startActivityForResult(Intent intent, int requestCode, Bundle options);
}
