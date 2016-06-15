// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_NATIVE_FILE_SYSTEM_EXTENSION_H_
#define XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_NATIVE_FILE_SYSTEM_EXTENSION_H_

#include <string>

#include "base/values.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace experimental {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class NativeFileSystemExtension : public XWalkExtension {
 public:
  explicit NativeFileSystemExtension(content::RenderProcessHost* host);
  ~NativeFileSystemExtension() override;

  // XWalkExtension implementation.
  XWalkExtensionInstance* CreateInstance() override;

 private:
  content::RenderProcessHost* host_;
};

class NativeFileSystemInstance : public XWalkExtensionInstance {
 public:
  explicit NativeFileSystemInstance(content::RenderProcessHost* host);

  // XWalkExtensionInstance implementation.
  void HandleMessage(std::unique_ptr<base::Value> msg) override;
  void HandleSyncMessage(std::unique_ptr<base::Value> msg) override;

 private:
  void OnRequestNativeFileSystem(std::unique_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
  content::RenderProcessHost* host_;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_NATIVE_FILE_SYSTEM_NATIVE_FILE_SYSTEM_EXTENSION_H_
