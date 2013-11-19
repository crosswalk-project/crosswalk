// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_DBUS_TEST_CLIENT_H_
#define XWALK_DBUS_TEST_CLIENT_H_

#include "base/memory/ref_counted.h"
#include "base/threading/thread.h"

namespace dbus {

class Bus;

// Helper class that provide code to create separated DBus thread and bus
// connection.
class TestClient {
 public:
  TestClient();
  ~TestClient();

 protected:
  scoped_refptr<Bus> bus_;

 private:
  base::Thread dbus_thread_;
};

}  // namespace dbus

#endif  // XWALK_DBUS_TEST_CLIENT_H_
