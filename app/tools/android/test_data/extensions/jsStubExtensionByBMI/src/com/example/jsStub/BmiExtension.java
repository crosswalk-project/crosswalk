// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.example.jsStub;

import org.xwalk.core.extension.*;

/*
 * An example for constructor extension
 */
public class BmiExtension extends XWalkExternalExtension {
    public BmiExtension(String extensionName, String jsApi, XWalkExtensionContextClient context) {
        super(extensionName, jsApi, context);
    }

    @JsConstructor(mainClass = BmiProcessor.class, isEntryPoint = true)
        public BmiProcessor onBmiProcessorInstance() {
            return new BmiProcessor();
        }
}
