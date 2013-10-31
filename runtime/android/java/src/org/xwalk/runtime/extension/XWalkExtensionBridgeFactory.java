// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

/**
 * A factory used to create the extension bridge.
 */
final class XWalkExtensionBridgeFactory {
    /**
     * Return a XWalkExtensionBridge instance for the given extension.
     */
    public static XWalkExtensionBridge createInstance(XWalkExtension extension) {
        return new XWalkCoreExtensionBridge(extension);
    }
}
