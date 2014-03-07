// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/debugger_tracker.h"

#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/devtools_client.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"

DebuggerTracker::DebuggerTracker(DevToolsClient* client) {
  client->AddListener(this);
}

DebuggerTracker::~DebuggerTracker() {}

Status DebuggerTracker::OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) {
  if (method == "Debugger.paused") {
    base::DictionaryValue empty_params;
    return client->SendCommand("Debugger.resume", empty_params);
  }
  return Status(kOk);
}
