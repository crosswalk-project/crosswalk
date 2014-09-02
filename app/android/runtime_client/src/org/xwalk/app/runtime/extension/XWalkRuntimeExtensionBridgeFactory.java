// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

/**
 * A factory used to create the extension bridge.
 */
final class XWalkRuntimeExtensionBridgeFactory {
    /**
     * Return a XWalkExtensionBridge instance for the given extension.
     */
    public static XWalkRuntimeExtensionBridge createInstance(XWalkExtensionClient extension) {
        return new XWalkCoreExtensionBridge(extension);
    }
}
