// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/win/xwalk_dotnet_extension.h"

#include "xwalk/extensions/common/win/xwalk_dotnet_bridge.h"
#include "xwalk/extensions/common/win/xwalk_dotnet_instance.h"

#include <string>
#include <vector>

namespace xwalk {
namespace extensions {

XWalkDotNetExtension::XWalkDotNetExtension(const base::FilePath& path)
  : initialized_(false),
    library_path_(path) {
  bridge_ = new XWalkDotNetBridge(this, library_path_.value());
  bridge_->set_name_callback(&set_name_callback);
  bridge_->set_javascript_api_callback(&set_javascript_api_callback);
}

XWalkDotNetExtension::~XWalkDotNetExtension() {
  if (!initialized_)
    return;
  delete bridge_;
}

bool XWalkDotNetExtension::Initialize() {
  if (initialized_)
    return true;

  if (!bridge_->Initialize())
    return false;

  initialized_ = true;
  return true;
}

XWalkExtensionInstance* XWalkDotNetExtension::CreateInstance() {
  return new XWalkDotNetInstance(this);
}

void XWalkDotNetExtension::set_name_callback(
  void* extension, const std::string& name) {
  if (extension) {
    xwalk::extensions::XWalkDotNetExtension* ext =
      reinterpret_cast<xwalk::extensions::XWalkDotNetExtension*>(extension);
    if (ext)
      ext->set_name(name);
  }
}

void XWalkDotNetExtension::set_javascript_api_callback(
  void* extension, const std::string& api) {
  if (extension) {
    xwalk::extensions::XWalkDotNetExtension* ext =
      reinterpret_cast<xwalk::extensions::XWalkDotNetExtension*>(extension);
    if (ext)
      ext->set_javascript_api(api);
  }
}

}  // namespace extensions
}  // namespace xwalk
