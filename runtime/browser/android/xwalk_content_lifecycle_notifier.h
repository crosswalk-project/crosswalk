// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENT_LIFECYCLE_NOTIFIER_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENT_LIFECYCLE_NOTIFIER_H_

#include "base/android/jni_android.h"
#include "base/macros.h"

namespace xwalk {

class XWalkContentLifecycleNotifier {
 public:
  static void OnXWalkViewCreated();
  static void OnXWalkViewDestroyed();

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(XWalkContentLifecycleNotifier);
};

bool RegisterXWalkContentLifecycleNotifier(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENT_LIFECYCLE_NOTIFIER_H_
