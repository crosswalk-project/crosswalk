// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/notify_done_forwarder.h"

#include "content/shell/shell_messages.h"
#include "content/shell/webkit_test_controller.h"

namespace content {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(NotifyDoneForwarder);

NotifyDoneForwarder::NotifyDoneForwarder(WebContents* web_contents)
    : WebContentsObserver(web_contents) {}

NotifyDoneForwarder::~NotifyDoneForwarder() {}

bool NotifyDoneForwarder::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(NotifyDoneForwarder, message)
    IPC_MESSAGE_HANDLER(ShellViewHostMsg_TestFinishedInSecondaryWindow,
                        OnTestFinishedInSecondaryWindow)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void NotifyDoneForwarder::OnTestFinishedInSecondaryWindow() {
  WebKitTestController::Get()->TestFinishedInSecondaryWindow();
}

}  // namespace content
