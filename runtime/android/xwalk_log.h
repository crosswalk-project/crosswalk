// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_ANDROID_XWALK_LOG_H_
#define XWALK_RUNTIME_ANDROID_XWALK_LOG_H_

#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, __FILE__, __VA_ARGS__)

#endif  // XWALK_RUNTIME_ANDROID_XWALK_LOG_H_
