// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.content.Intent;

/**
 * Interface for bridging XWalkExtension functionalities to its backend implementation.
 */
interface XWalkExternalExtensionBridge {
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

    public void postBinaryMessage(int instanceId, byte[] message);

    /**
     * Broadcast a message frome native side to all receivers on JavaScript side.
     *
     * @param message The message content to be posted.
     */
    public void broadcastMessage(String message);

    /**
     * Called when a new extension instance is created.
     * Handle the message from JavaScript side to native side.
     *
     * @param instanceId The extension instance id.
     */
    public void onInstanceCreated(int instanceId);

    /**
     * Called when a extension instance is destroyed.
     *
     * @param instanceId The extension instance id.
     */
    public void onInstanceDestroyed(int instanceId);

    /**
     * Handle the message from JavaScript side to native side.
     *
     * @param instanceId The extension instance id.
     * @param message The message content received on native side.
     */
    public void onMessage(int instanceId, String message);

    /**
     * Handle the binary message from JavaScript side to native side.
     * JavaScript wraps the binary message into an ArrayBuffer.
     *
     * @param instanceId The extension instance id.
     * @param message The binary message content received on native side.
     */
    public void onBinaryMessage(int instanceId, byte[] message);

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
    public String onSyncMessage(int instanceId, String message);

    /**
     * Called when the extension is required to be started.
     */
    public void onStart();

    /**
     * Called when the extension is required to be resumed.
     */
    public void onResume();

    /**
     * Called when the extension is required to be paused.
     */
    public void onPause();

    /**
     * Called when the extension is required to be stopped.
     */
    public void onStop();


    /**
     * Called when the extension is required to be destroyed.
     *
     * It gives a chance to cleanup the resource allocated for the extension.
     * Any attempt to call a method on XWalkExtension after onDestroy is called
     * will cause undefined program behavior.
     */
    public void onDestroy();

    /**
     * Called when the extension's activity is required to be invoked but
     * without creating a new activity instance.
     */
    public void onNewIntent(Intent intent);
}
