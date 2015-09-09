// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_INSTANCE_H_
#define XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_INSTANCE_H_

#include "xwalk/extensions/common/xwalk_extension.h"

#include <string>

namespace xwalk {
namespace extensions {

class XWalkDotNetExtension;

class XWalkDotNetInstance : public XWalkExtensionInstance {
 public:
  explicit XWalkDotNetInstance(XWalkDotNetExtension* extension);
  ~XWalkDotNetInstance() override;
  bool isValid() const { return instance_dotnet_ && extension_; }
 private:
  // XWalkExtensionInstance implementation.
  void HandleMessage(scoped_ptr<base::Value> msg) override;
  void HandleSyncMessage(scoped_ptr<base::Value> msg) override;
  static void PostMessageToJSCallback(
    void* instance, const std::string& message);
  static void SetSyncReply(void* instance, const std::string& message);

  friend class XWalkDotNetExtensionTest;
  XWalkDotNetExtension* extension_;
  void* instance_dotnet_;
  DISALLOW_COPY_AND_ASSIGN(XWalkDotNetInstance);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_WIN_XWALK_DOTNET_INSTANCE_H_
