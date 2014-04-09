// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_app_extension_bridge.h"

#include <string>

#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"

#if defined(OS_LINUX)
#include "xwalk/application/browser/application_system_linux.h"
#include "xwalk/application/browser/application_service_provider_linux.h"
#include "xwalk/application/browser/linux/running_application_object.h"
#endif

namespace xwalk {

XWalkAppExtensionBridge::XWalkAppExtensionBridge()
    : app_system_(NULL) {
}

XWalkAppExtensionBridge::~XWalkAppExtensionBridge() {}

void XWalkAppExtensionBridge::CheckAPIAccessControl(
    int render_process_id,
    const std::string& extension_name,
    const std::string& api_name,
    const extensions::PermissionCallback& callback) {
  CHECK(app_system_);
  application::ApplicationService *service =
      app_system_->application_service();

  application::Application *app =
      service->GetApplicationByRenderHostID(render_process_id);
  if (!app) {
    callback.Run(extensions::UNDEFINED_RUNTIME_PERM);
    return;
  }

  service->CheckAPIAccessControl(app->id(), extension_name, api_name,
      *(reinterpret_cast<const xwalk::application::PermissionCallback*>
      (&callback)));
}

bool XWalkAppExtensionBridge::RegisterPermissions(
    int render_process_id,
    const std::string& extension_name,
    const std::string& perm_table) {
  CHECK(app_system_);
  application::ApplicationService *service =
      app_system_->application_service();
  application::Application *app =
      service->GetApplicationByRenderHostID(render_process_id);
  if (!app)
    return false;

  return service->RegisterPermissions(app->id(), extension_name, perm_table);
}

void XWalkAppExtensionBridge::ExtensionProcessCreated(
    int render_process_id,
    const IPC::ChannelHandle& channel_handle) {
#if defined(OS_LINUX)
  CHECK(app_system_);
  application::ApplicationService* service = app_system_->application_service();
  application::Application* app =
      service->GetApplicationByRenderHostID(render_process_id);
  CHECK(app);

  application::ApplicationSystemLinux* app_system =
      static_cast<application::ApplicationSystemLinux*>(app_system_);
  application::RunningApplicationObject* running_app_object =
      app_system->service_provider()->GetRunningApplicationObject(app);
  CHECK(running_app_object);
  running_app_object->ExtensionProcessCreated(channel_handle);
#endif
}

}  // namespace xwalk
