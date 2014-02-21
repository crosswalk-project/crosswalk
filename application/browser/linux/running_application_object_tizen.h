// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATION_OBJECT_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATION_OBJECT_TIZEN_H_

#include "xwalk/application/browser/linux/running_application_object.h"

namespace xwalk {
namespace application {

class TizenRunningApplicationObject : public RunningApplicationObject {
 public:
  TizenRunningApplicationObject(scoped_refptr<dbus::Bus> bus,
                                const std::string& app_id,
                                const std::string& launcher_name,
                                Application* application);

 private:
  // Handlers for running app interface.
  void OnHide(dbus::MethodCall* method_call,
              dbus::ExportedObject::ResponseSender response_sender);

  // Handlers for app cmd forwarder interface.
  void OnLaunchCmd(dbus::MethodCall* method_call,
                   dbus::ExportedObject::ResponseSender response_sender);
  void OnExitCmd(dbus::MethodCall* method_call,
                 dbus::ExportedObject::ResponseSender response_sender);
  void OnKillCmd(dbus::MethodCall* method_call,
                 dbus::ExportedObject::ResponseSender response_sender);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATION_OBJECT_TIZEN_H_
