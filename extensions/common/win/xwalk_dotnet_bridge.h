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

#include <string>

#include "base/macros.h"

namespace xwalk {
namespace extensions {

typedef void(*set_name_callback_func)(void*, const std::string&);
typedef void(*set_javacript_api_callback_func)(void*, const std::string&);
typedef void(*post_message_callback_func)(void*, const std::string&);
typedef void(*set_sync_reply_callback_func)(void*, const std::string&);

class DOTNET_BRIDGE_EXPORT XWalkDotNetBridge {
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

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_BRIDGE_H_

