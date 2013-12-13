// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_VECTOR_H_
#define XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_VECTOR_H_

#include <vector>

namespace xwalk {
namespace extensions {

class XWalkExtension;

typedef std::vector<XWalkExtension*> XWalkExtensionVector;

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_XWALK_EXTENSION_VECTOR_H_
