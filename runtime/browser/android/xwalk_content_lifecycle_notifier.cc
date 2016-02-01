// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_content_lifecycle_notifier.h"

#include "jni/XWalkContentLifecycleNotifier_jni.h"

using base::android::AttachCurrentThread;

namespace xwalk {

bool RegisterXWalkContentLifecycleNotifier(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

// static
void XWalkContentLifecycleNotifier::OnXWalkViewCreated() {
  Java_XWalkContentLifecycleNotifier_onXWalkViewCreated(AttachCurrentThread());
}

// static
void XWalkContentLifecycleNotifier::OnXWalkViewDestroyed() {
  Java_XWalkContentLifecycleNotifier_onXWalkViewDestroyed(
      AttachCurrentThread());
}

}  // namespace xwalk
