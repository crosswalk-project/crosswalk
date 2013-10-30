// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENT_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENT_H_

#include "base/android/jni_helper.h"
#include "base/android/scoped_java_ref.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/runtime/browser/android/renderer_host/xwalk_render_view_host_ext.h"

using base::android::ScopedJavaLocalRef;

namespace content {
class BrowserContext;
class WebContents;
}

namespace xwalk {

class XWalkWebContentsDelegate;
class XWalkContentsClientBridge;

class XWalkContent {
 public:
  XWalkContent(JNIEnv* env, jobject obj, jobject web_contents_delegate,
      jobject contents_client_bridge);
  ~XWalkContent();

  static XWalkContent* FromWebContents(content::WebContents* web_contents);

  jint GetWebContents(JNIEnv* env, jobject obj, jobject delegate);
  void ClearCache(JNIEnv* env, jobject obj, jboolean include_disk_files);
  ScopedJavaLocalRef<jstring> DevToolsAgentId(JNIEnv* env, jobject obj);
  void Destroy(JNIEnv* env, jobject obj);
  ScopedJavaLocalRef<jstring> GetVersion(JNIEnv* env, jobject obj);

  XWalkRenderViewHostExt* render_view_host_ext() {
    return render_view_host_ext_.get();
  };

  void SetJsOnlineProperty(JNIEnv* env, jobject obj, jboolean network_up);
  jboolean SetManifest(JNIEnv* env,
                       jobject obj,
                       jstring path,
                       jstring manifest);

 private:
  content::WebContents* CreateWebContents(JNIEnv* env, jobject delegate);

  JavaObjectWeakGlobalRef java_ref_;
  scoped_ptr<content::WebContents> web_contents_;
  scoped_ptr<XWalkWebContentsDelegate> web_contents_delegate_;
  scoped_ptr<XWalkRenderViewHostExt> render_view_host_ext_;
  scoped_ptr<XWalkContentsClientBridge> contents_client_bridge_;
};

bool RegisterXWalkContent(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENT_H_
