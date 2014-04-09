// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATION_OBJECT_H_
#define XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATION_OBJECT_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "dbus/bus.h"
#include "ipc/ipc_channel_handle.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/dbus/object_manager_adaptor.h"

namespace xwalk {
namespace application {

// Represents the running application inside D-Bus hierarchy of
// RunningApplicationsManager.
//
// Watches for the D-Bus presence of the launcher, when it disappears (e.g. the
// launcher was ended by a task manager) will terminate the corresponding
// application in Crosswalk.
class RunningApplicationObject : public dbus::ManagedObject {
 public:
  RunningApplicationObject(scoped_refptr<dbus::Bus> bus,
                           const std::string& app_id,
                           const std::string& launcher_name,
                           Application* application);

  virtual ~RunningApplicationObject();

  void ExtensionProcessCreated(const IPC::ChannelHandle& handle);

 private:
  void TerminateApplication(Application::TerminationMode mode);

  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success);

  void OnTerminate(Application::TerminationMode termination_mode,
                   dbus::MethodCall* method_call,
                   dbus::ExportedObject::ResponseSender response_sender);

  void OnGetExtensionProcessChannel(
      dbus::MethodCall* method_call,
      dbus::ExportedObject::ResponseSender response_sender);

  void ListenForOwnerChange();
  void UnlistenForOwnerChange();
  void OnNameOwnerChanged(const std::string& service_owner);

  void OnLauncherDisappeared();

  scoped_refptr<dbus::Bus> bus_;
  std::string launcher_name_;
  dbus::Bus::GetServiceOwnerCallback owner_change_callback_;
  Application* application_;

  IPC::ChannelHandle ep_bp_channel_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_LINUX_RUNNING_APPLICATION_OBJECT_H_

