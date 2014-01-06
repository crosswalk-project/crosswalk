// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_app_extension_bridge.h"

#include <string>

#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"

namespace xwalk {

XWalkAppExtensionBridge::XWalkAppExtensionBridge(
      application::ApplicationSystem* app_system)
  : app_system_(app_system) {
}

XWalkAppExtensionBridge::~XWalkAppExtensionBridge() {}

void XWalkAppExtensionBridge::CheckAPIAccessControl(std::string extension_name,
    std::string api_name, const extensions::PermissionCallback& callback) {
  xwalk::application::ApplicationService* service =
      app_system_->application_service();
  // TODO(Bai): The extension system should be aware where the request is
  // comming from, i.e. the request origin application ID. So, apart from
  // the rp-ep mapping, we need an additional mapping for AppID-rp.
  const ScopedVector<xwalk::application::Application> &apps =
      service->active_applications();
  if (apps.empty()) {
    callback.Run(extensions::INVALID_RUNTIME_PERM);
    return;
  }
  std::string app_id = (apps.front())->id();
  service->CheckAPIAccessControl(app_id, extension_name, api_name,
     *(reinterpret_cast<const xwalk::application::PermissionCallback*>
      (&callback)));
}

bool XWalkAppExtensionBridge::RegisterPermissions(std::string extension_name,
    std::string perm_table) {
  xwalk::application::ApplicationService* service =
        app_system_->application_service();
  const ScopedVector<xwalk::application::Application> &apps =
      service->active_applications();
  if (apps.empty()) {
    return false;
  }
  std::string app_id = (apps.front())->id();
  return service->RegisterPermissions(app_id, extension_name, perm_table);
}

}  // namespace xwalk

