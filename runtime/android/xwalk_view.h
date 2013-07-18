// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_ANDROID_XWALK_VIEW_H_
#define XWALK_RUNTIME_ANDROID_XWALK_VIEW_H_

#include <jni.h>

namespace content {
class WebContents;
}

namespace xwalk {

bool RegisterXWalkView(JNIEnv* env);
void InitWithWebContents(content::WebContents *web_contents);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_ANDROID_XWALK_VIEW_H_

