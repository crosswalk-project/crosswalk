// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_XWALK_TIZEN_IMPL_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_XWALK_TIZEN_IMPL_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/xwalkdriver/xwalk/xwalk_impl.h"

class DevToolsHttpClient;

class TizenDevice {
 public:
  TizenDevice() {}
  ~TizenDevice() {}
};

class XwalkTizenImpl : public XwalkImpl {
 public:
  XwalkTizenImpl(
      scoped_ptr<DevToolsHttpClient> client,
      ScopedVector<DevToolsEventListener>& devtools_event_listeners,
      scoped_ptr<PortReservation> port_reservation,
      scoped_ptr<TizenDevice> device);
  virtual ~XwalkTizenImpl();

  // Overridden from Xwalk
  virtual std::string GetOperatingSystemName() OVERRIDE;

  // Overridden from XwalkImpl:
  virtual Status QuitImpl() OVERRIDE;

 private:
  scoped_ptr<TizenDevice> device_;
};

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_XWALK_TIZEN_IMPL_H_
