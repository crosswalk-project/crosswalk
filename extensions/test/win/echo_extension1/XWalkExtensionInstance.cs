// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System;

namespace xwalk
{
public class XWalkExtensionInstance
{
  public XWalkExtensionInstance(dynamic native) {
    native_ = native;
  }

  public void HandleMessage(String message) {
    native_.PostMessageToJS(message);
  }
  public void HandleSyncMessage(String message) {
    native_.SendSyncReply(message);
  }
  private dynamic native_;
}
}
