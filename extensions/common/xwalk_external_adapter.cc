// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_external_adapter.h"

#include "base/logging.h"

namespace xwalk {
namespace extensions {

XWalkExternalAdapter::XWalkExternalAdapter()
    : next_xw_extension_(1),
      next_xw_instance_(1) {}

XWalkExternalAdapter::~XWalkExternalAdapter() {}

XWalkExternalAdapter* XWalkExternalAdapter::GetInstance() {
  return Singleton<XWalkExternalAdapter>::get();
}

XW_Extension XWalkExternalAdapter::GetNextXWExtension() {
  return next_xw_extension_++;
}

XW_Instance XWalkExternalAdapter::GetNextXWInstance() {
  return next_xw_instance_++;
}

void XWalkExternalAdapter::RegisterExtension(
    XWalkExternalExtension* extension) {
  XW_Extension xw_extension = extension->xw_extension_;
  CHECK(IsValidXWExtension(xw_extension));
  CHECK(extension_map_.find(xw_extension) == extension_map_.end());
  extension_map_[xw_extension] = extension;
}

void XWalkExternalAdapter::UnregisterExtension(
    XWalkExternalExtension* extension) {
  XW_Extension xw_extension = extension->xw_extension_;
  CHECK(IsValidXWExtension(xw_extension));
  CHECK(extension_map_.find(xw_extension) != extension_map_.end());
  extension_map_.erase(xw_extension);
}

void XWalkExternalAdapter::RegisterInstance(XWalkExternalInstance* context) {
  XW_Instance xw_instance = context->xw_instance_;
  CHECK(IsValidXWInstance(xw_instance));
  CHECK(instance_map_.find(xw_instance) == instance_map_.end());
  instance_map_[xw_instance] = context;
}

void XWalkExternalAdapter::UnregisterInstance(XWalkExternalInstance* context) {
  XW_Instance xw_instance = context->xw_instance_;
  CHECK(IsValidXWInstance(xw_instance));
  CHECK(instance_map_.find(xw_instance) != instance_map_.end());
  instance_map_.erase(xw_instance);
}

const void* XWalkExternalAdapter::GetInterface(const char* name) {
  if (!strcmp(name, XW_CORE_INTERFACE_1)) {
    static const XW_CoreInterface_1 coreInterface1 = {
      CoreSetExtensionName,
      CoreSetJavaScriptAPI,
      CoreRegisterInstanceCallbacks,
      CoreRegisterShutdownCallback,
      CoreSetInstanceData,
      CoreGetInstanceData
    };
    return &coreInterface1;
  }

  if (!strcmp(name, XW_MESSAGING_INTERFACE_1)) {
    static const XW_MessagingInterface_1 messagingInterface1 = {
      MessagingRegister,
      MessagingPostMessage
    };
    return &messagingInterface1;
  }

  if (!strcmp(name, XW_INTERNAL_SYNC_MESSAGING_INTERFACE_1)) {
    static const XW_Internal_SyncMessagingInterface_1
        syncMessagingInterface1 = {
      SyncMessagingRegister,
      SyncMessagingSetSyncReply
    };
    return &syncMessagingInterface1;
  }

  if (!strcmp(name, XW_INTERNAL_ENTRY_POINTS_INTERFACE_1)) {
    static const XW_Internal_EntryPointsInterface_1 entryPointsInterface1 = {
      EntryPointsSetExtraJSEntryPoints
    };
    return &entryPointsInterface1;
  }

  LOG(WARNING) << "Interface '" << name << "' is not supported.";
  return NULL;
}

bool XWalkExternalAdapter::IsValidXWExtension(XW_Extension xw_extension) {
  return xw_extension > 0 && xw_extension < next_xw_extension_;
}

bool XWalkExternalAdapter::IsValidXWInstance(XW_Instance xw_instance) {
  return xw_instance > 0 && xw_instance < next_xw_instance_;
}

XWalkExternalExtension* XWalkExternalAdapter::GetExtension(
    XW_Extension xw_extension) {
  XWalkExternalAdapter* adapter = XWalkExternalAdapter::GetInstance();
  ExtensionMap::iterator it = adapter->extension_map_.find(xw_extension);
  if (it == adapter->extension_map_.end())
    return NULL;
  return it->second;
}

XWalkExternalInstance* XWalkExternalAdapter::GetInstance(
    XW_Instance xw_instance) {
  XWalkExternalAdapter* adapter = XWalkExternalAdapter::GetInstance();
  InstanceMap::iterator it = adapter->instance_map_.find(xw_instance);
  if (it == adapter->instance_map_.end())
    return NULL;
  return it->second;
}

// static
void XWalkExternalAdapter::LogInvalidCall(
    int32_t value, const char* type,
    const char* interface, const char* function) {
  LOG(WARNING) << "Ignoring call to " << interface << " function " << function
               << " as it received wrong XW_" << type << "=" << value << ".";
}

}  // namespace extensions
}  // namespace xwalk
