// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_content.h"

#include "base/android/jni_string.h"
#include "base/base_paths_android.h"
#include "base/path_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/devtools_agent_host.h"
#include "components/navigation_interception/intercept_navigation_delegate.h"
#include "xwalk/runtime/browser/android/net_disk_cache_remover.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge_base.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_delegate.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "jni/XWalkContent_jni.h"

using base::android::ScopedJavaLocalRef;
using navigation_interception::InterceptNavigationDelegate;

namespace xwalk {

namespace {

const void* kXWalkContentUserDataKey = &kXWalkContentUserDataKey;

class XWalkContentUserData : public base::SupportsUserData::Data {
 public:
  explicit XWalkContentUserData(XWalkContent* ptr) : content_(ptr) {}

  static XWalkContent* GetContents(content::WebContents* web_contents) {
    if (!web_contents)
      return NULL;
    XWalkContentUserData* data = reinterpret_cast<XWalkContentUserData*>(
        web_contents->GetUserData(kXWalkContentUserDataKey));
    return data ? data->content_ : NULL;
  }

 private:
  XWalkContent* content_;
};

}  // namespace

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

// static
XWalkContent* XWalkContent::FromWebContents(
    content::WebContents* web_contents) {
  return XWalkContentUserData::GetContents(web_contents);
}

jint XWalkContent::GetWebContents(
    JNIEnv* env, jobject obj, jobject intercept_navigation_delegate) {
  if (!web_contents_) {
    web_contents_.reset(CreateWebContents(env, intercept_navigation_delegate));

    render_view_host_ext_.reset(
        new XWalkRenderViewHostExt(web_contents_.get()));
  }
  return reinterpret_cast<jint>(web_contents_.get());
}

content::WebContents* XWalkContent::CreateWebContents(
    JNIEnv* env, jobject intercept_navigation_delegate) {
  RuntimeContext* runtime_context =
      XWalkContentBrowserClient::GetRuntimeContext();
  content::WebContents* web_contents = content::WebContents::Create(
      content::WebContents::CreateParams(runtime_context));

  web_contents->SetUserData(kXWalkContentUserDataKey,
                            new XWalkContentUserData(this));

  XWalkContentsClientBridgeBase::Associate(web_contents,
      contents_client_bridge_.get());
  InterceptNavigationDelegate::Associate(web_contents,
      make_scoped_ptr(new InterceptNavigationDelegate(
          env, intercept_navigation_delegate)));
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

ScopedJavaLocalRef<jstring> XWalkContent::DevToolsAgentId(JNIEnv* env,
                                                          jobject obj) {
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  scoped_refptr<content::DevToolsAgentHost> agent_host(
      content::DevToolsAgentHost::GetOrCreateFor(rvh));
  return base::android::ConvertUTF8ToJavaString(env, agent_host->GetId());
}

void XWalkContent::Destroy(JNIEnv* env, jobject obj) {
  delete this;
}

ScopedJavaLocalRef<jstring> XWalkContent::GetVersion(JNIEnv* env,
                                                     jobject obj) {
  return base::android::ConvertUTF8ToJavaString(env, XWALK_VERSION);
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
