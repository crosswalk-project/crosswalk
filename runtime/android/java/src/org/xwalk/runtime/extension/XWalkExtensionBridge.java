// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.content.Intent;

/**
 * Interface for bridging XWalkExtension functionalities to its backend implementation.
 */
public interface XWalkExtensionBridge {
    /**
     * Post a message from native to a specific receiver on JavaScript side.
     *
     * A receiver on JavaScript side is considered as an extension instance created
     * for a JavaScript context. If a web page has multiple JavaScript contexts,
     * e.g. iframes, multiple extension instances will be created for each JavaScript
     * context, and an integer is assigned to each extension instance as the unique
     * identifier.
     *
     * @param instanceId The internal unique id on native side to identify the message
     *                   receiver. Always got from handleMessage interface.
     * @param message The message content to be posted.
     */
    public void postMessage(int instanceId, String message);

    /**
     * Broadcast a message frome native side to all receivers on JavaScript side.
     *
     * @param message The message content to be posted.
     */
    public void broadcastMessage(String message);

    /**
     * Handle the message from JavaScript side to native side.
     *
     * @param instanceId The extension instance id.
     * @param message The message content received on native side.
     */
    public void handleMessage(int instanceId, String message);

    /**
     * Handle the message from JavaScript side to native side in a synchronous way.
     *
     * Note that it will block the current thread until the method is finished and
     * returns the result as string.
     *
     * @param instanceId The extension instance id.
     * @param message The message content received on native side.
     *
     * @return The result to be posted to JavaScript side
     */
    public String handleSyncMessage(int instanceId, String message);

    /**
     * Called when the extension is required to be resumed.
     */
    public void onResume();

    /**
     * Called when the extension is required to be paused.
     */
    public void onPause();

    /**
     * Called when the extension is required to be destroyed.
     *
     * It gives a chance to cleanup the resource allocated for the extension.
     * Any attempt to call a method on XWalkExtension after onDestroy is called
     * will cause undefined program behavior.
     */
    public void onDestroy();

    /**
     * Called when the extension exists if activity launched exists.
     * TODO(hmin): Figure out if it is necessary and how to use it.
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data);
}
