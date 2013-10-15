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
  explicit RawSocketExtension();
  virtual ~RawSocketExtension();

  // XWalkExtension implementation.
  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;
};

class RawSocketInstance : public XWalkExtensionInstance {
 public:
  RawSocketInstance();

  // XWalkExtensionInstance implementation.
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  void AddBindingObject(const std::string& object_id,
                        scoped_ptr<BindingObject> obj);

 private:
  void OnTCPServerSocketConstructor(
      scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnTCPSocketConstructor(scoped_ptr<XWalkExtensionFunctionInfo> info);

  XWalkExtensionFunctionHandler handler_;
  BindingObjectStore store_;
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_EXTENSION_H_
