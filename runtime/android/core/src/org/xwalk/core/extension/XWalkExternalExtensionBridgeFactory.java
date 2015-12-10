// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

/**
 * A factory used to create the extension bridge.
 */
final class XWalkExternalExtensionBridgeFactory {
    /**
     * Return a XWalkExtensionBridge instance for the given extension.
     */
    public static XWalkExternalExtensionBridge createInstance(XWalkExternalExtension extension) {
        return new XWalkCoreExtensionBridge(extension);
    }
}
