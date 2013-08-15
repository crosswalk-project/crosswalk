// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_
#define XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_

#include <string>
#include "xwalk/extensions/browser/xwalk_extension_internal.h"

namespace xwalk {

using extensions::XWalkExtension;
using extensions::XWalkInternalExtension;

class RuntimeExtension : public XWalkInternalExtension {
 public:
  RuntimeExtension();

  virtual const char* GetJavaScriptAPI() OVERRIDE;

  virtual XWalkExtension::Context* CreateContext(
      const XWalkExtension::PostMessageCallback& post_message) OVERRIDE;

  class RuntimeContext : public XWalkInternalExtension::InternalContext {
   public:
    explicit RuntimeContext(
        const XWalkExtension::PostMessageCallback& post_message);

   private:
    void OnGetAPIVersion(const std::string& function_name,
                         const std::string& callback_id, base::ListValue* args);
  };
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_EXTENSION_RUNTIME_EXTENSION_H_
