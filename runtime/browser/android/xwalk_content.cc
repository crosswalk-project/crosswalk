// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_content.h"

#include "base/base_paths_android.h"
#include "base/path_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/runtime/browser/android/net_disk_cache_remover.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge_base.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_delegate.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "jni/XWalkContent_jni.h"

namespace xwalk {

XWalkContent::XWalkContent(JNIEnv* env,
                           jobject obj,
                           jobject web_contents_delegate,
                           jobject contents_client_bridge)
    : java_ref_(env, obj),
      web_contents_delegate_(
          new XWalkWebContentsDelegate(env, web_contents_delegate)),
      contents_client_bridge_(
          new XWalkContentsClientBridge(env, contents_client_bridge)) {
}

XWalkContent::~XWalkContent() {
}

jint XWalkContent::GetWebContents(JNIEnv* env, jobject obj) {
  if (!web_contents_) {
    web_contents_.reset(CreateWebContents());

    render_view_host_ext_.reset(
        new XWalkRenderViewHostExt(web_contents_.get()));
  }
  return reinterpret_cast<jint>(web_contents_.get());
}

content::WebContents* XWalkContent::CreateWebContents() {
  RuntimeContext* runtime_context =
      XWalkContentBrowserClient::GetRuntimeContext();
  content::WebContents* web_contents = content::WebContents::Create(
      content::WebContents::CreateParams(runtime_context));

  XWalkContentsClientBridgeBase::Associate(web_contents,
      contents_client_bridge_.get());
  web_contents->SetDelegate(web_contents_delegate_.get());
  return web_contents;
}

void XWalkContent::ClearCache(
    JNIEnv* env,
    jobject obj,
    jboolean include_disk_files) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  render_view_host_ext_->ClearCache();

  if (include_disk_files) {
    RemoveHttpDiskCache(web_contents_->GetBrowserContext(),
                        web_contents_->GetRoutingID());
  }
}

void XWalkContent::Destroy(JNIEnv* env, jobject obj) {
  delete this;
}

static jint Init(JNIEnv* env, jobject obj, jobject web_contents_delegate,
    jobject contents_client_bridge) {
  XWalkContent* xwalk_core_content =
    new XWalkContent(env, obj, web_contents_delegate, contents_client_bridge);
  return reinterpret_cast<jint>(xwalk_core_content);
}

bool RegisterXWalkContent(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
