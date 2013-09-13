// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_EXTENSION_H_
#define XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_EXTENSION_H_

#include <string>
#include "base/values.h"
#include "xwalk/extensions/browser/xwalk_extension_internal.h"
#include "xwalk/sysapps/common/binding_object_store.h"

namespace xwalk {
namespace sysapps {

using extensions::XWalkExtension;
using extensions::XWalkExtensionInstance;
using extensions::XWalkInternalExtension;

class RawSocketExtension : public XWalkInternalExtension {
 public:
  explicit RawSocketExtension();
  virtual ~RawSocketExtension();

  // XWalkExtension implementation.
  virtual const char* GetJavaScriptAPI() OVERRIDE;
  virtual XWalkExtensionInstance* CreateInstance(
    const PostMessageCallback& post_message) OVERRIDE;

 private:
  std::string api;
};

class RawSocketInstance : public BindingObjectStore {
 public:
  RawSocketInstance(const XWalkExtension::PostMessageCallback& post_message);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_RAW_SOCKET_RAW_SOCKET_EXTENSION_H_
