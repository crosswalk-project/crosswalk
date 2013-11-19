// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_DBUS_DBUS_MANAGER_H_
#define XWALK_DBUS_DBUS_MANAGER_H_

#include <string>
#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"

namespace base {
class Thread;
}

namespace dbus {
class Bus;
}

namespace xwalk {

// Holds a DBus thread and a session bus connection, that should be shared by
// all users of DBus inside Crosswalk.
class DBusManager {
 public:
  explicit DBusManager(const std::string& service_name);
  ~DBusManager();

  // FIXME(cmarcelo): Notify about failure. We probably want to move
  // to a delegate/observer interface in the future.
  void Initialize(const base::Closure& on_success_callback);

  scoped_refptr<dbus::Bus> session_bus();

 private:
  void OnNameOwned(const std::string& service_name, bool success);

  base::WeakPtrFactory<DBusManager> weak_factory_;
  std::string service_name_;
  scoped_ptr<base::Thread> dbus_thread_;
  scoped_refptr<dbus::Bus> session_bus_;
  base::Closure on_success_callback_;
};

}  // namespace xwalk

#endif  // XWALK_DBUS_DBUS_MANAGER_H_
