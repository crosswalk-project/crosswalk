// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_content.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "base/android/jni_string.h"
#include "base/base_paths_android.h"
#include "base/json/json_reader.h"
#include "base/path_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"
#include "components/navigation_interception/intercept_navigation_delegate.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
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

  // XWalk does not use disambiguation popup for multiple targets.
  content::RendererPreferences* prefs =
      web_contents->GetMutableRendererPrefs();
  prefs->tap_multiple_targets_strategy =
      content::TAP_MULTIPLE_TARGETS_STRATEGY_NONE;

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

void XWalkContent::SetJsOnlineProperty(JNIEnv* env,
                                       jobject obj,
                                       jboolean network_up) {
  render_view_host_ext_->SetJsOnlineProperty(network_up);
}

/*
bool XWalkContent::OnReceivedHttpAuthRequest(const JavaRef<jobject>& handler,
                                             const std::string& host,
                                             const std::string& realm) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return false;

  ScopedJavaLocalRef<jstring> jhost = ConvertUTF8ToJavaString(env, host);
  ScopedJavaLocalRef<jstring> jrealm = ConvertUTF8ToJavaString(env, realm);
  Java_XWalkContent_onReceivedHttpAuthRequest(env, obj.obj(), handler.obj(),
      jhost.obj(), jrealm.obj());
  return true;
}
*/

jboolean XWalkContent::SetManifest(JNIEnv* env,
                                   jobject obj,
                                   jstring path,
                                   jstring manifest_string) {
  std::string path_str = base::android::ConvertJavaStringToUTF8(env, path);
  std::string json_input =
      base::android::ConvertJavaStringToUTF8(env, manifest_string);

  base::Value* manifest_value = base::JSONReader::Read(json_input);
  if (!manifest_value) return false;

  base::DictionaryValue* manifest_dictionary;
  manifest_value->GetAsDictionary(&manifest_dictionary);
  if (!manifest_dictionary) return false;

  scoped_ptr<base::DictionaryValue>
      manifest_dictionary_ptr(manifest_dictionary);

  xwalk::application::Manifest manifest(
      xwalk::application::Manifest::INVALID_TYPE,
      manifest_dictionary_ptr.Pass());

  std::string url;
  if (manifest.GetString(
          xwalk::application_manifest_keys::kLaunchLocalPathKey, &url)) {
    // According to original proposal for "app:launch:local_path", the "http"
    // and "https" schemes are supported. So |url| should do nothing when it
    // already has "http" or "https" scheme.
    std::string lower_url = url;
    std::transform(lower_url.begin(), lower_url.end(),
                   lower_url.begin(), std::tolower);
    if (lower_url.find(content::kHttpScheme) == std::string::npos &&
        lower_url.find(content::kHttpsScheme) == std::string::npos) {
      url = path_str + url;
    }
  } else {
    manifest.GetString(
        xwalk::application_manifest_keys::kLaunchWebURLKey, &url);
  }

  ScopedJavaLocalRef<jstring> buffer =
      base::android::ConvertUTF8ToJavaString(env, url);
  Java_XWalkContent_onGetUrlFromManifest(env, obj, buffer.obj());
  return true;
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
