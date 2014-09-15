// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_MANAGER_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_MANAGER_H_

#include <string>
#include <vector>
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionManager {
 public:
  static XWalkExtensionManager* Get();
  // XWalkExtensionAndroid needs to register its extensions on
  // XWalkBrowserMainParts(which owns XWalkExtensonManager
  // so they get correctly registered on-demand-
  // by XWalkExtensionService each time a in_process Server is created.
  void RegisterExtension(XWalkExtension* pExtension);
  // Lookup the extension with the given name from the extension list that is
  // already registered. Returns NULL if no such extension exists.
  XWalkExtension* LookupExtension(const std::string& name);
  // For iterate the extensions
  XWalkExtensionVector::const_iterator Begin();
  XWalkExtensionVector::const_iterator End();

 protected:
  static XWalkExtensionManager* pXWalkExtensionManager;
  XWalkExtensionVector extensions_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_MANAGER_H_
