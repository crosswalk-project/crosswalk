// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension.h"

namespace xwalk {
namespace extensions {

XWalkExtension::XWalkExtension() {}

XWalkExtension::~XWalkExtension() {}

XWalkExtension::Context::Context(const PostMessageCallback& post_message)
    : post_message_(post_message) {
}

XWalkExtension::Context::~Context() {}

}  // namespace extensions
}  // namespace xwalk
