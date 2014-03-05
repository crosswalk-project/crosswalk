// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/chrome/xwalk_existing_impl.h"
#include "xwalk/test/chromedriver/chrome/devtools_http_client.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/net/port_server.h"

XwalkExistingImpl::XwalkExistingImpl(
    scoped_ptr<DevToolsHttpClient> client,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners)
    : ChromeImpl(client.Pass(),
                 devtools_event_listeners,
                 scoped_ptr<PortReservation>()) {}

XwalkExistingImpl::~XwalkExistingImpl() {}

std::string XwalkExistingImpl::GetOperatingSystemName() {
  return std::string();
}

Status XwalkExistingImpl::QuitImpl() {
  return Status(kOk);
}

