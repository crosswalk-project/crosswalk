// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_XWALK_ANDROID_IMPL_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_XWALK_ANDROID_IMPL_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/xwalkdriver/xwalk/xwalk_impl.h"

class Device;
class DevToolsHttpClient;

class XwalkAndroidImpl : public XwalkImpl {
 public:
  XwalkAndroidImpl(
      scoped_ptr<DevToolsHttpClient> client,
      ScopedVector<DevToolsEventListener>& devtools_event_listeners,
      scoped_ptr<PortReservation> port_reservation,
      scoped_ptr<Device> device);
  virtual ~XwalkAndroidImpl();

  // Overridden from Xwalk
  virtual std::string GetOperatingSystemName() OVERRIDE;

  // Overridden from XwalkImpl:
  virtual Status QuitImpl() OVERRIDE;

 private:
  scoped_ptr<Device> device_;
};

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_XWALK_ANDROID_IMPL_H_
