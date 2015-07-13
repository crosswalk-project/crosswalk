// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.xwalk.core.internal.extensions.XWalkExtensionAndroid;

/**
 * This class represents an extension and could be implemented by callers.
 */
@XWalkAPI
public abstract class XWalkExtensionInternal extends XWalkExtensionAndroid {
    /**
     * Constructor with name and javascript API.
     * @param name  the exposed namespace.
     * @param jsApi the string of javascript API.
     * @since 2.1
     */
    @XWalkAPI
    public XWalkExtensionInternal(String name, String jsApi) {
        super(name, jsApi);
    }

    /**
     * Constructor with name, javascript API and entry points.
     * @param name the exposed namespace.
     * @param jsApi the string of javascript API.
     * @param entryPoints Entry points are used when the extension needs to
     *                    have objects outside the namespace that is
     *                    implicitly created using its name.
     * @since 2.1
     */
    @XWalkAPI
    public XWalkExtensionInternal(String name, String jsApi, String[] entryPoints) {
        super(name, jsApi, entryPoints);
    }

    /**
     * Destroy an extension.
     */
    protected void destroyExtension() {
        super.destroyExtension();
    }

    /**
     * Send message to an instance.
     * @param instanceID the id of instance.
     * @param message the message.
     * @since 2.1
     */
    @XWalkAPI
    public void postMessage(int instanceID, String message) {
        super.postMessage(instanceID, message);
    }

    /**
     * Send binary message to an instance.
     * @param instanceID the id of instance.
     * @param message the binary message.
     * @since 6.0
     */
    @XWalkAPI
    public void postBinaryMessage(int instanceID, byte[] message) {
        super.postBinaryMessage(instanceID, message);
    }

    /**
     * Broadcast message to all extension instances.
     * @param message the message.
     * @since 2.1
     */
    @XWalkAPI
    public void broadcastMessage(String message) {
        super.broadcastMessage(message);
    }

    /**
     * Notify the extension that an instance is created.
     * @param instanceID the id of instance.
     * @since 15.45
     */
    @XWalkAPI
    public void onInstanceCreated(int instanceID) {}

    /**
     * Notify the extension that an instance is destroyed.
     * @param instanceID the id of instance.
     * @since 15.45
     */
    @XWalkAPI
    public void onInstanceDestroyed(int instanceID) {}

    /**
     * Notify the extension that the async message is received.
     * @param instanceID the id of instance.
     * @param message the received message.
     * @since 2.1
     */
    @XWalkAPI
    public abstract void onMessage(int instanceID, String message);

    /**
     * Notify the extension that the async binary message is received.
     * @param instanceID the id of instance.
     * @param message the received binar message.
     * @since 6.0
     */
    @XWalkAPI
    public void onBinaryMessage(int instanceID, byte[] message) {}

    /**
     * Notify the extension that the sync message is received.
     * @param instanceID the id of instance.
     * @param message the received message.
     * @since 2.1
     */
    @XWalkAPI
    public abstract String onSyncMessage(int instanceID, String message);
}
