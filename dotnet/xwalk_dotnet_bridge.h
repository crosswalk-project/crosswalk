// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_DOTNET_XWALK_DOTNET_BRIDGE_H_
#define XWALK_DOTNET_XWALK_DOTNET_BRIDGE_H_

#include <string>

#include "xwalk/extensions/public/XW_Extension.h"
#include "xwalk/extensions/public/XW_Extension_EntryPoints.h"
#include "xwalk/extensions/public/XW_Extension_Message_2.h"
#include "xwalk/extensions/public/XW_Extension_Permissions.h"
#include "xwalk/extensions/public/XW_Extension_Runtime.h"
#include "xwalk/extensions/public/XW_Extension_SyncMessage.h"

namespace xwalk {
namespace extensions {

struct PrivateBridgeData;
typedef void* XWalkExtensionDotNetInstance;

class XWalkDotNetBridge {
 public:
  XWalkDotNetBridge();
  ~XWalkDotNetBridge();

  // XW_Extension callbacks.
  static void OnShutdown(XW_Extension xw_extension);
  static void OnInstanceCreated(XW_Instance xw_instance);
  static void OnInstanceDestroyed(XW_Instance xw_instance);
  static void HandleMessage(XW_Instance xw_instance, const char* msg);
  static void HandleSyncMessage(XW_Instance xw_instance, const char* msg);

  bool Initialize();
  XWalkExtensionDotNetInstance CreateInstance(XW_Instance native_instance);
  void PostBinaryMessageToInstance(XW_Instance instance, const char* message, const size_t size);
  void PostMessageToInstance(XW_Instance instance, const std::string& message);
  void SetSyncReply(XW_Instance instance, const std::string& message);

 private:
  PrivateBridgeData* private_data_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_DOTNET_XWALK_DOTNET_BRIDGE_H_
