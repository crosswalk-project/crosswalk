// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_LINUX_DBUS_OBJECT_MANAGER_H_
#define XWALK_APPLICATION_TOOLS_LINUX_DBUS_OBJECT_MANAGER_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "dbus/bus.h"
#include "dbus/exported_object.h"
#include "dbus/message.h"
#include "dbus/object_manager.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"
#include "dbus/property.h"

class DBusObjectManager : public dbus::ObjectManager::Interface {
 public:
  class Observer {
   public:
    virtual void OnEPChannelCreated() = 0;

   protected:
    virtual ~Observer() {}
  };

  DBusObjectManager(dbus::Bus* bus, base::MessageLoop* main_loop);

  bool Launch(const std::string& appid_or_url, int launcher_pid,
              bool fullscreen, bool remote_debugging);
  std::pair<std::string, int> GetEPChannel() const;
  bool Suspend();
  bool Resume();

  bool IsApplicationRunning(const std::string& app_id);

  void SetObserver(Observer* observer) { observer_ = observer; }

 private:
  void OnOwnershipCallback(const std::string& service_name, bool success);
  void ObjectAdded(const dbus::ObjectPath& object_path,
                   const std::string& interface_name) override;
  void ObjectRemoved(const dbus::ObjectPath& object_path,
                     const std::string& interface_name) override;
  dbus::PropertySet* CreateProperties(
      dbus::ObjectProxy* object_proxy,
      const dbus::ObjectPath& object_path,
      const std::string& interface_name) override;

  void OnPropertyChanged(const dbus::ObjectPath& object_path,
                         const std::string& name);

  void ConnectToApplicationManager();
  void ConnectToApplicationSignal(const std::string& signal_name);
  void OnAppSignal(dbus::Signal* signal);
  void OnAppSignalConnected(const std::string& interface_name,
                            const std::string& signal_name,
                            bool success);

  scoped_refptr<dbus::Bus> bus_;
  dbus::ObjectManager* running_apps_manager_;
  dbus::ObjectProxy* running_proxy_;
  dbus::ObjectProxy* app_proxy_;

  // this is needed for exit events which come via dbus interface
  base::MessageLoop* main_loop_;

  base::WeakPtrFactory<DBusObjectManager> weak_ptr_factory_;

  Observer* observer_;

  DISALLOW_COPY_AND_ASSIGN(DBusObjectManager);
};

#endif  // XWALK_APPLICATION_TOOLS_LINUX_DBUS_OBJECT_MANAGER_H_
