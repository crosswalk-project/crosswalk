// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_manager.h"
#include <string.h>
#include "base/logging.h"

namespace xwalk {
namespace extensions {

XWalkExtensionManager* XWalkExtensionManager::pXWalkExtensionManager = NULL;

XWalkExtensionManager* XWalkExtensionManager:: Get() {
  if (NULL == pXWalkExtensionManager)
    pXWalkExtensionManager = new XWalkExtensionManager();
  return pXWalkExtensionManager;
}

void XWalkExtensionManager::RegisterExtension(XWalkExtension* extension) {
// Since the creation of extension object is driven by Java side, and each
// Java extension is backed by a native extension object. However, the Java
// object may be destroyed by Android lifecycle management without destroying
// the native side object. We keep the reference to native extension object
// to make sure we can reuse the native object if Java extension is re-created
// on resuming.
  extensions_.push_back(extension);
}

XWalkExtension* XWalkExtensionManager::LookupExtension(
    const std::string& name) {
  XWalkExtensionVector::const_iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    XWalkExtension* extension = *it;
    if (name == extension->name()) return extension;
  }
  return NULL;
}

XWalkExtensionVector::const_iterator XWalkExtensionManager::Begin() {
  return extensions_.begin();
}

XWalkExtensionVector::const_iterator XWalkExtensionManager::End() {
  return extensions_.end();
}


}  // namespace extensions
}  // namespace xwalk
