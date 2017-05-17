// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/xwalk_android_impl.h"

#include "xwalk/test/xwalkdriver/xwalk/device_manager.h"
#include "xwalk/test/xwalkdriver/xwalk/devtools_http_client.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/net/port_server.h"

XwalkAndroidImpl::XwalkAndroidImpl(
    scoped_ptr<DevToolsHttpClient> client,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<PortReservation> port_reservation,
    scoped_ptr<Device> device)
    : XwalkImpl(client.Pass(),
                 devtools_event_listeners,
                 port_reservation.Pass()),
      device_(device.Pass()) {}

XwalkAndroidImpl::~XwalkAndroidImpl() {}

std::string XwalkAndroidImpl::GetOperatingSystemName() {
  return "ANDROID";
}

Status XwalkAndroidImpl::QuitImpl() {
  return device_->TearDown();
}

