// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_BRIDGE_H_
#define XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_BRIDGE_H_

#ifdef DOTNET_BRIDGE_IMPLEMENTATION
#define DOTNET_BRIDGE_EXPORT __declspec(dllexport)
#else
#define DOTNET_BRIDGE_EXPORT __declspec(dllimport)
#endif

#include <memory>
#include <string>

#include "base/macros.h"

namespace xwalk {
namespace extensions {

typedef void(*set_name_callback_func)(void*, const std::string&);
typedef void(*set_javacript_api_callback_func)(void*, const std::string&);
typedef void(*post_message_callback_func)(void*, const std::string&);
typedef void(*set_sync_reply_callback_func)(void*, const std::string&);

class XWalkDotNetBridge {
 public:
  XWalkDotNetBridge(void* extension, const std::wstring& library_path);
  ~XWalkDotNetBridge();

  bool Initialize();
  void* CreateInstance(void* native_instance);
  void HandleMessage(void* instance, const std::string& message);
  void HandleSyncMessage(void* instance, const std::string& message);
  void PostMessageToInstance(void* instance, const std::string& message);
  void SetSyncReply(void* instance, const std::string& message);

  void set_name_callback(set_name_callback_func);
  void set_javascript_api_callback(set_javacript_api_callback_func);
  void set_post_message_callback(post_message_callback_func);
  void set_set_sync_reply_callback(set_sync_reply_callback_func);

 private:
  void* extension_;
  std::wstring library_path_;
  bool initialized_;
  void* extension_dotnet_;
  void* extension_assembly_;
  set_name_callback_func set_name_callback_;
  set_javacript_api_callback_func set_javacript_api_callback_;
  post_message_callback_func post_message_callback_;
  set_sync_reply_callback_func set_sync_reply_callback_;
  DISALLOW_COPY_AND_ASSIGN(XWalkDotNetBridge);
};

extern "C" DOTNET_BRIDGE_EXPORT void* CreateDotNetBridge(
  void* extension, const std::wstring& library_path_) {
  return new XWalkDotNetBridge(extension, library_path_);
}

extern "C" DOTNET_BRIDGE_EXPORT void ReleaseDotNetBridge(void* bridge) {
  if (!bridge)
    return;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  delete bridge_obj;
}

extern "C" DOTNET_BRIDGE_EXPORT bool InitializeBridge(void* bridge) {
  if (!bridge)
    return false;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  return bridge_obj->Initialize();
}

extern "C" DOTNET_BRIDGE_EXPORT void* CreateDotNetInstance(
  void* bridge, void* native_instance) {
  if (!bridge)
    return 0;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  return bridge_obj->CreateInstance(native_instance);
}

extern "C" DOTNET_BRIDGE_EXPORT void HandleMessage(
  void* bridge, void* instance, const std::string& message) {
  if (!bridge)
    return;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  return bridge_obj->HandleMessage(instance, message);
}

extern "C" DOTNET_BRIDGE_EXPORT void HandleSyncMessage(
  void* bridge, void* instance, const std::string& message) {
  if (!bridge)
    return;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  return bridge_obj->HandleSyncMessage(instance, message);
}

extern "C" DOTNET_BRIDGE_EXPORT void set_name_callback(
  void* bridge, set_name_callback_func func) {
  if (!bridge)
    return;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  bridge_obj->set_name_callback(func);
}

extern "C" DOTNET_BRIDGE_EXPORT void set_javascript_api_callback(
  void* bridge, set_javacript_api_callback_func func) {
  if (!bridge)
    return;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  bridge_obj->set_javascript_api_callback(func);
}

extern "C" DOTNET_BRIDGE_EXPORT void set_post_message_callback(
  void* bridge, post_message_callback_func func) {
  if (!bridge)
    return;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  bridge_obj->set_post_message_callback(func);
}

extern "C" DOTNET_BRIDGE_EXPORT void set_set_sync_reply_callback(
  void* bridge, set_sync_reply_callback_func func) {
  if (!bridge)
    return;
  XWalkDotNetBridge* bridge_obj = reinterpret_cast<XWalkDotNetBridge*>(bridge);
  bridge_obj->set_set_sync_reply_callback(func);
}

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_BRIDGE_H_
