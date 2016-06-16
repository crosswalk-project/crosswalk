// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_EXTENSION_H_
#define XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_EXTENSION_H_

#include <string>
#include "base/values.h"
#include "xwalk/sysapps/common/binding_object_store.h"

namespace xwalk {
namespace sysapps {

using extensions::XWalkExtension;
using extensions::XWalkExtensionFunctionHandler;
using extensions::XWalkExtensionFunctionInfo;
using extensions::XWalkExtensionInstance;

class RawSocketExtension : public XWalkExtension {
 public:
  RawSocketExtension();
  ~RawSocketExtension() override;

  // XWalkExtension implementation.
  XWalkExtensionInstance* CreateInstance() override;
};

class RawSocketInstance : public XWalkExtensionInstance {
 public:
  RawSocketInstance();

  // XWalkExtensionInstance implementation.
  void HandleMessage(std::unique_ptr<base::Value> msg) override;

  void AddBindingObject(const std::string& object_id,
                        std::unique_ptr<BindingObject> obj);

 private:
  void OnTCPServerSocketConstructor(
      std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnTCPSocketConstructor(std::unique_ptr<XWalkExtensionFunctionInfo> info);
  void OnUDPSocketConstructor(std::unique_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
  BindingObjectStore store_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_EXTENSION_H_
