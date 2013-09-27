// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_V8_UTILS_H_
#define XWALK_EXTENSIONS_RENDERER_XWALK_V8_UTILS_H_

#include <string>

namespace v8 {

class TryCatch;

}

namespace xwalk {
namespace extensions {

// Helper function that makes v8 exceptions human readable.
std::string ExceptionToString(const v8::TryCatch& try_catch);

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_V8_UTILS_H_
