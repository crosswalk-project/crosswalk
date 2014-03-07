// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/xwalk_existing_impl.h"
#include "xwalk/test/xwalkdriver/xwalk/devtools_http_client.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/net/port_server.h"

XwalkExistingImpl::XwalkExistingImpl(
    scoped_ptr<DevToolsHttpClient> client,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners)
    : XwalkImpl(client.Pass(),
                 devtools_event_listeners,
                 scoped_ptr<PortReservation>()) {}

XwalkExistingImpl::~XwalkExistingImpl() {}

std::string XwalkExistingImpl::GetOperatingSystemName() {
  return std::string();
}

Status XwalkExistingImpl::QuitImpl() {
  return Status(kOk);
}

