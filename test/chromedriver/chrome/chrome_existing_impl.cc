// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/chrome/chrome_existing_impl.h"
#include "xwalk/test/chromedriver/chrome/devtools_http_client.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/net/port_server.h"

ChromeExistingImpl::ChromeExistingImpl(
    scoped_ptr<DevToolsHttpClient> client,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners)
    : ChromeImpl(client.Pass(),
                 devtools_event_listeners,
                 scoped_ptr<PortReservation>()) {}

ChromeExistingImpl::~ChromeExistingImpl() {}

std::string ChromeExistingImpl::GetOperatingSystemName() {
 return std::string();
}

Status ChromeExistingImpl::QuitImpl() {
  return Status(kOk);
}

