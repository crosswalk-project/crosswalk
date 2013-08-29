// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_SETTINGS_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_SETTINGS_H_

#include <jni.h>

#include "base/android/jni_helper.h"
#include "base/android/scoped_java_ref.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_observer.h"

namespace xwalk {

class XWalkRenderViewHostExt;

class XWalkSettings : public content::WebContentsObserver {
 public:
  XWalkSettings(JNIEnv* env, jobject obj);
  virtual ~XWalkSettings();

  // Called from Java.
  void Destroy(JNIEnv* env, jobject obj);
  void ResetScrollAndScaleState(JNIEnv* env, jobject obj);
  void SetWebContents(JNIEnv* env, jobject obj, jint web_contents);
  void UpdateEverything(JNIEnv* env, jobject obj);
  void UpdateInitialPageScale(JNIEnv* env, jobject obj);
  void UpdateUserAgent(JNIEnv* env, jobject obj);
  void UpdateWebkitPreferences(JNIEnv* env, jobject obj);

 private:
  struct FieldIds;

  XWalkRenderViewHostExt* GetXWalkRenderViewHostExt();
  void UpdateEverything();
  void UpdatePreferredSizeMode();

  // WebContentsObserver overrides:
  virtual void RenderViewCreated(
      content::RenderViewHost* render_view_host) OVERRIDE;

  // Java field references for accessing the values in the Java object.
  scoped_ptr<FieldIds> field_ids_;

  JavaObjectWeakGlobalRef xwalk_settings_;
};

bool RegisterXWalkSettings(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_SETTINGS_H_
