// Copyright (c) 2015 Intel Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_NATIVE_EXTENSION_LOADER_ANDROID_H_
#define XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_NATIVE_EXTENSION_LOADER_ANDROID_H_

#include <jni.h>

namespace xwalk {
namespace extensions {

bool RegisterXWalkNativeExtensionLoaderAndroid(JNIEnv* env);

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_NATIVE_EXTENSION_LOADER_ANDROID_H_
