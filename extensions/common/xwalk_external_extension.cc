// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_external_extension.h"

#include <string>
#include <vector>
#include "base/logging.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "xwalk/extensions/common/xwalk_external_adapter.h"

namespace xwalk {
namespace extensions {

XWalkExternalExtension::XWalkExternalExtension(const base::FilePath& path)
    : xw_extension_(0),
      created_instance_callback_(NULL),
      destroyed_instance_callback_(NULL),
      shutdown_callback_(NULL),
      handle_msg_callback_(NULL),
      handle_sync_msg_callback_(NULL),
      initialized_(false) {
  std::string error;
  base::ScopedNativeLibrary library(base::LoadNativeLibrary(path, &error));
  if (!library.is_valid()) {
    LOG(WARNING) << "Error loading extension '" << path.AsUTF8Unsafe() << "': "
                 << error;
    return;
  }

  XW_Initialize_Func initialize = reinterpret_cast<XW_Initialize_Func>(
      library.GetFunctionPointer("XW_Initialize"));
  if (!initialize) {
    LOG(WARNING) << "Error loading extension '" << path.AsUTF8Unsafe() << "': "
                 << "couldn't get XW_Initialize function.";
    return;
  }

  XWalkExternalAdapter* external_adapter = XWalkExternalAdapter::GetInstance();
  xw_extension_ = external_adapter->GetNextXWExtension();
  external_adapter->RegisterExtension(this);
  int ret = initialize(xw_extension_, XWalkExternalAdapter::GetInterface);
  if (ret != XW_OK) {
    LOG(WARNING) << "Error loading extension '" << path.AsUTF8Unsafe() << "': "
                 << "XW_Initialize function returned error value.";
    return;
  }

  library_.Reset(library.Release());
  initialized_ = true;
}

XWalkExternalExtension::~XWalkExternalExtension() {
  if (!initialized_)
    return;

  if (shutdown_callback_)
    shutdown_callback_(xw_extension_);
  XWalkExternalAdapter::GetInstance()->UnregisterExtension(this);
}

bool XWalkExternalExtension::is_valid() {
  return initialized_;
}

XWalkExtensionInstance* XWalkExternalExtension::CreateInstance() {
  XW_Instance xw_instance =
      XWalkExternalAdapter::GetInstance()->GetNextXWInstance();
  return new XWalkExternalInstance(this, xw_instance);
}

#define RETURN_IF_INITIALIZED(FUNCTION)                          \
  if (initialized_) {                                            \
    LOG(WARNING) << "Error: can't call " FUNCTION                \
                 << " for extension '" << this->name() << "'"    \
                 << " after XW_Initialize returned.";            \
    return;                                                      \
  }

void XWalkExternalExtension::CoreSetExtensionName(const char* name) {
  RETURN_IF_INITIALIZED("SetExtensionName from CoreInterface");
  set_name(name);
}

void XWalkExternalExtension::CoreSetJavaScriptAPI(const char* js_api) {
  RETURN_IF_INITIALIZED("SetJavaScriptAPI from CoreInterface");
  set_javascript_api(std::string(js_api));
}

void XWalkExternalExtension::CoreRegisterInstanceCallbacks(
    XW_CreatedInstanceCallback created_callback,
    XW_DestroyedInstanceCallback destroyed_callback) {
  RETURN_IF_INITIALIZED("RegisterInstanceCallbacks from CoreInterface");
  created_instance_callback_ = created_callback;
  destroyed_instance_callback_ = destroyed_callback;
}

void XWalkExternalExtension::CoreRegisterShutdownCallback(
    XW_ShutdownCallback callback) {
  RETURN_IF_INITIALIZED("RegisterShutdownCallbacks from CoreInterface");
  shutdown_callback_ = callback;
}

void XWalkExternalExtension::MessagingRegister(
    XW_HandleMessageCallback callback) {
  RETURN_IF_INITIALIZED("Register from MessagingInterface");
  handle_msg_callback_ = callback;
}

void XWalkExternalExtension::SyncMessagingRegister(
    XW_HandleSyncMessageCallback callback) {
  RETURN_IF_INITIALIZED("Register from Internal_SyncMessagingInterface");
  handle_sync_msg_callback_ = callback;
}

void XWalkExternalExtension::EntryPointsSetExtraJSEntryPoints(
    const char** entry_points) {
  RETURN_IF_INITIALIZED("SetExtraJSEntryPoints from EntryPoints");
  if (!entry_points)
    return;

  std::vector<std::string> entries;
  for (int i = 0; entry_points[i]; ++i)
    entries.push_back(std::string(entry_points[i]));

  set_entry_points(entries);
}

bool XWalkExternalExtension::PermissionsCheckAPIAccessControl(
    const char* app_id, const char* api_name) {
  bool result = CheckAPIAccessControl(std::string(app_id),
      std::string(api_name));
  VLOG(0) << "CoreCheckAPIAccessControl result: " << result;
  return result;
}


}  // namespace extensions
}  // namespace xwalk
