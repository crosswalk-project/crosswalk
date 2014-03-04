// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/devtools_event_listener.h"

#include "chrome/test/chromedriver/chrome/status.h"

DevToolsEventListener::~DevToolsEventListener() {}

Status DevToolsEventListener::OnConnected(DevToolsClient* client) {
  return Status(kOk);
}

Status DevToolsEventListener::OnEvent(DevToolsClient* client,
                                      const std::string& method,
                                      const base::DictionaryValue& params) {
  return Status(kOk);
}

Status DevToolsEventListener::OnCommandSuccess(
    DevToolsClient* client,
    const std::string& method) {
  return Status(kOk);
}
