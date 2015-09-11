// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_EXTENSION_H_
#define XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_EXTENSION_H_


#include <string>
#include "base/files/file_path.h"
#include "base/values.h"
#include "base/scoped_native_library.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace base {
class FilePath;
};

namespace xwalk {
namespace extensions {

class XWalkDotNetBridge;

// XWalkDotNetExtension implements an XWalkExtension backed by a shared
// library implemented in .NET with few defined entry points.
class XWalkDotNetExtension : public XWalkExtension {
 public:
  explicit XWalkDotNetExtension(const base::FilePath& path);

  ~XWalkDotNetExtension();

  bool Initialize();

  XWalkDotNetBridge* GetBridge() const { return bridge_; }

  void set_runtime_variables(const base::ValueMap& runtime_variables) {
    runtime_variables_ = runtime_variables;
  }

 protected:
  // XWalkExtension implementation.
  XWalkExtensionInstance* CreateInstance() override;

 private:
  // Variables from the browser process. Usually things like currently-running
  // application ID.
  base::ValueMap runtime_variables_;

  static void set_name_callback(void* extension, const std::string& name);
  static void set_javascript_api_callback(
    void* extension, const std::string& api);

  base::FilePath library_path_;
  XWalkDotNetBridge* bridge_;

  bool initialized_;

  DISALLOW_COPY_AND_ASSIGN(XWalkDotNetExtension);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_EXTENSION_H_
