// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/wifidirect_component_win.h"

#include "base/path_service.h"
#include "base/win/windows_version.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/extensions/common/xwalk_external_extension.h"

namespace {

using WiFiDirect = std::unique_ptr<xwalk::extensions::XWalkExternalExtension>;

WiFiDirect createWiFiDirectExtension() {
  if (base::win::GetVersion() < base::win::VERSION_WIN10)
    return nullptr;
  base::FilePath exe_path;
  if (!PathService::Get(base::DIR_EXE, &exe_path)) {
    LOG(WARNING) << "Failed to get exe path when initializing "
      << "wifidirect_extension_bridge.dll.";
    return nullptr;
  }
  const base::FilePath extensionPath = exe_path.Append(L"wifidirect_extension_bridge.dll");

  std::unique_ptr<base::DictionaryValue::Storage> runtime_variables_(
      new base::DictionaryValue::Storage);
  (*runtime_variables_)["extension_path"] =
      base::WrapUnique(new base::StringValue(extensionPath.AsUTF8Unsafe()));
  WiFiDirect extension(
    new xwalk::extensions::XWalkExternalExtension(extensionPath));
  extension->set_runtime_variables(runtime_variables_.get());

  if (!extension->Initialize()) {
    LOG(WARNING) << "Failed to initialize extension: "
      << "wifidirect_extension_bridge.dll";
    return nullptr;
  }
  return extension;
}

} // namespace

namespace xwalk {

void WiFiDirectComponent::CreateExtensionThreadExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
  if (WiFiDirect wifiDirect = createWiFiDirectExtension())
    extensions->push_back(wifiDirect.release());
}

}  // namespace xwalk
