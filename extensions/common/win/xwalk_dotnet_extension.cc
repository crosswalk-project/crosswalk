// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/win/xwalk_dotnet_extension.h"

#include <vector>

#include "base/logging.h"
#include "xwalk/extensions/common/win/xwalk_dotnet_instance.h"

namespace {
  typedef void(*set_name_callback_func)(void*, const std::string&);
  typedef void(*set_javacript_api_callback_func)(void*, const std::string&);
  typedef void* (CALLBACK* CreateDotNetBridge)(void*, const std::wstring&);
  typedef void (CALLBACK* ReleaseDotNetBridge)(void*);
  typedef bool (CALLBACK* InitializeBridge)(void*);
  typedef void (CALLBACK* set_name_callback)(void*, set_name_callback_func);
  typedef void (CALLBACK* set_javascript_api_callback)(
    void*, set_javacript_api_callback_func);
}  // anonymous namespace

namespace xwalk {
namespace extensions {

XWalkDotNetExtension::XWalkDotNetExtension(const base::FilePath& path)
  : library_path_(path),
    bridge_(0),
    initialized_(false) {
  dotnet_bridge_handle_ = LoadLibrary(L"xwalk_dotnet_bridge.dll");
  if (!dotnet_bridge_handle_) {
    LOG(WARNING) << "Could not find the .NET bridge, no .NET extensions"
      << "are going load.";
      return;
  }

  CreateDotNetBridge create_dotnet_ptr =
    (CreateDotNetBridge)GetProcAddress(dotnet_bridge_handle_,
                                       "CreateDotNetBridge");
  if (!create_dotnet_ptr)
    return;

  bridge_ = create_dotnet_ptr(this, library_path_.value());

  set_name_callback set_name_callback_ptr =
    (set_name_callback)GetProcAddress(dotnet_bridge_handle_,
                                       "set_name_callback");
  set_name_callback_ptr(bridge_, &SetNameCallback);

  set_javascript_api_callback set_javascript_api_callback_ptr =
    (set_javascript_api_callback)GetProcAddress(dotnet_bridge_handle_,
    "set_javascript_api_callback");
  set_javascript_api_callback_ptr(bridge_, &SetJavascriptAPICallback);
}

XWalkDotNetExtension::~XWalkDotNetExtension() {
  if (!initialized_)
    return;

  if (dotnet_bridge_handle_) {
    ReleaseDotNetBridge release_dotnet_ptr =
      (ReleaseDotNetBridge)GetProcAddress(dotnet_bridge_handle_,
      "ReleaseDotNetBridge");
    release_dotnet_ptr(bridge_);
    FreeLibrary(dotnet_bridge_handle_);
  }
}

bool XWalkDotNetExtension::Initialize() {
  if (initialized_)
    return true;

  if (!bridge_)
    return false;

  InitializeBridge initialize_bridge_ptr =
    (InitializeBridge)GetProcAddress(dotnet_bridge_handle_,
    "InitializeBridge");

  if (!initialize_bridge_ptr(bridge_))
    return false;

  initialized_ = true;
  return true;
}

XWalkExtensionInstance* XWalkDotNetExtension::CreateInstance() {
  return new XWalkDotNetInstance(this);
}

void XWalkDotNetExtension::SetNameCallback(
    void* extension, const std::string& name) {
  if (!extension)
    return;

  xwalk::extensions::XWalkDotNetExtension* ext =
    reinterpret_cast<xwalk::extensions::XWalkDotNetExtension*>(extension);
  if (ext)
    ext->set_name(name);
}

void XWalkDotNetExtension::SetJavascriptAPICallback(
    void* extension, const std::string& api) {
  if (!extension)
    return;

  xwalk::extensions::XWalkDotNetExtension* ext =
    reinterpret_cast<xwalk::extensions::XWalkDotNetExtension*>(extension);
  if (ext)
    ext->set_javascript_api(api);
}

}  // namespace extensions
}  // namespace xwalk
