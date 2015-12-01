// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.example.jsStub;

import org.xwalk.app.runtime.extension.*;

public class EchoExtension extends XWalkExtensionClient {

  public EchoExtension(String extensionName, String jsApi, XWalkExtensionContextClient context) {
    super(extensionName, jsApi, context);
  }
  
  @JsConstructor(isEntryPoint = true, mainClass = Echo.class)
  public Echo onEcho() {
      return new Echo();
  }
}
