// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_content.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/base_paths_android.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/path_service.h"
#include "base/pickle.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"
#include "components/navigation_interception/intercept_navigation_delegate.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/runtime/browser/android/net_disk_cache_remover.h"
#include "xwalk/runtime/browser/android/state_serializer.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge_base.h"
#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client_impl.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_delegate.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate_android.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "jni/XWalkContent_jni.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaLocalRef;
using content::BrowserThread;
using content::WebContents;
using navigation_interception::InterceptNavigationDelegate;
using xwalk::application_manifest_keys::kDisplay;

namespace keys = xwalk::application_manifest_keys;

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
XWalkContent* XWalkContent::FromID(int render_process_id,
                                   int render_view_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const content::RenderViewHost* rvh =
      content::RenderViewHost::FromID(render_process_id, render_view_id);
  if (!rvh) return NULL;
  content::WebContents* web_contents =
      content::WebContents::FromRenderViewHost(rvh);
  if (!web_contents) return NULL;
  return FromWebContents(web_contents);
}

// static
XWalkContent* XWalkContent::FromWebContents(
    content::WebContents* web_contents) {
  return XWalkContentUserData::GetContents(web_contents);
}

jlong XWalkContent::GetWebContents(
    JNIEnv* env, jobject obj, jobject io_thread_client,
    jobject intercept_navigation_delegate) {
  if (!web_contents_) {
    web_contents_.reset(CreateWebContents(env, io_thread_client,
                                          intercept_navigation_delegate));

    render_view_host_ext_.reset(
        new XWalkRenderViewHostExt(web_contents_.get()));
  }
  return reinterpret_cast<intptr_t>(web_contents_.get());
}

content::WebContents* XWalkContent::CreateWebContents(
    JNIEnv* env, jobject io_thread_client,
    jobject intercept_navigation_delegate) {

  RuntimeContext* runtime_context =
      XWalkRunner::GetInstance()->runtime_context();
  CHECK(runtime_context);

  content::WebContents* web_contents = content::WebContents::Create(
      content::WebContents::CreateParams(runtime_context));

  web_contents->SetUserData(kXWalkContentUserDataKey,
                            new XWalkContentUserData(this));

  XWalkContentsIoThreadClientImpl::RegisterPendingContents(web_contents);

  // XWalk does not use disambiguation popup for multiple targets.
  content::RendererPreferences* prefs =
      web_contents->GetMutableRendererPrefs();
  prefs->tap_multiple_targets_strategy =
      content::TAP_MULTIPLE_TARGETS_STRATEGY_NONE;

  XWalkContentsClientBridgeBase::Associate(web_contents,
      contents_client_bridge_.get());
  XWalkContentsIoThreadClientImpl::Associate(web_contents,
      ScopedJavaLocalRef<jobject>(env, io_thread_client));
  int child_id = web_contents->GetRenderProcessHost()->GetID();
  int route_id = web_contents->GetRoutingID();
  RuntimeResourceDispatcherHostDelegateAndroid::OnIoThreadClientReady(
      child_id, route_id);
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
  if (manifest.GetString(keys::kLaunchLocalPathKey, &url)) {
    // According to original proposal for "app:launch:local_path", the "http"
    // and "https" schemes are supported. So |url| should do nothing when it
    // already has "http" or "https" scheme.
    std::string scheme = GURL(url).scheme();
    if (scheme != url::kHttpScheme && scheme != url::kHttpsScheme)
      url = path_str + url;
  } else {
    manifest.GetString(keys::kLaunchWebURLKey, &url);
  }

  std::string match_patterns;
  const base::ListValue* xwalk_hosts = NULL;
  if (manifest.GetList(
          xwalk::application_manifest_keys::kXWalkHostsKey, &xwalk_hosts)) {
      base::JSONWriter::Write(xwalk_hosts, &match_patterns);
  }
  render_view_host_ext_->SetOriginAccessWhitelist(url, match_patterns);

  std::string csp;
  manifest.GetString(keys::kCSPKey, &csp);
  RuntimeContext* runtime_context =
      XWalkRunner::GetInstance()->runtime_context();
  CHECK(runtime_context);
  runtime_context->SetCSPString(csp);

  ScopedJavaLocalRef<jstring> url_buffer =
      base::android::ConvertUTF8ToJavaString(env, url);

  if (manifest.HasPath(kDisplay)) {
    std::string display_string;
    manifest.GetString(kDisplay, &display_string);
    // TODO(David): update the handling process of the display strings
    // including fullscreen etc.
    bool display_as_fullscreen = (
        display_string.find("fullscreen") != std::string::npos);
    Java_XWalkContent_onGetFullscreenFlagFromManifest(
        env, obj, display_as_fullscreen ? JNI_TRUE : JNI_FALSE);
  }

  // Check whether need to display launch screen. (Read from manifest.json)
  if (manifest.HasPath(keys::kLaunchScreen)) {
    std::string ready_when;
    // Get the value of 'ready_when' from manifest.json
    manifest.GetString(keys::kLaunchScreenReadyWhen, &ready_when);
    ScopedJavaLocalRef<jstring> ready_when_buffer =
        base::android::ConvertUTF8ToJavaString(env, ready_when);

    // Get the value of 'image_border'
    // 1. When 'launch_screen.[orientation]' was defined, but no 'image_border'
    //    The value of 'image_border' will be set as 'empty'.
    // 2. Otherwise, there is no 'launch_screen.[orientation]' defined,
    //    The value of 'image_border' will be empty.
    const char empty[] = "empty";
    std::string image_border_default;
    manifest.GetString(keys::kLaunchScreenImageBorderDefault,
                       &image_border_default);
    if (image_border_default.empty() && manifest.HasPath(
        keys::kLaunchScreenDefault)) {
      image_border_default = empty;
    }

    std::string image_border_landscape;
    manifest.GetString(keys::kLaunchScreenImageBorderLandscape,
                       &image_border_landscape);
    if (image_border_landscape.empty() && manifest.HasPath(
        keys::kLaunchScreenLandscape)) {
      image_border_landscape = empty;
    }

    std::string image_border_portrait;
    manifest.GetString(keys::kLaunchScreenImageBorderPortrait,
                       &image_border_portrait);
    if (image_border_portrait.empty() && manifest.HasPath(
        keys::kLaunchScreenPortrait)) {
      image_border_portrait = empty;
    }

    std::string image_border = image_border_default + ';' +
        image_border_landscape  + ';' + image_border_portrait;
    ScopedJavaLocalRef<jstring> image_border_buffer =
        base::android::ConvertUTF8ToJavaString(env, image_border);

    Java_XWalkContent_onGetUrlAndLaunchScreenFromManifest(
        env, obj, url_buffer.obj(), ready_when_buffer.obj(),
        image_border_buffer.obj());
  } else {
    // No need to display launch screen, load the url directly.
    Java_XWalkContent_onGetUrlFromManifest(env, obj, url_buffer.obj());
  }
  return true;
}

jint XWalkContent::GetRoutingID(JNIEnv* env, jobject obj) {
  DCHECK(web_contents_.get());
  return web_contents_->GetRoutingID();
}

base::android::ScopedJavaLocalRef<jbyteArray> XWalkContent::GetState(
    JNIEnv* env,
    jobject obj) {
  if (!web_contents_->GetController().GetEntryCount())
    return ScopedJavaLocalRef<jbyteArray>();

  Pickle pickle;
  if (!WriteToPickle(*web_contents_, &pickle)) {
    return ScopedJavaLocalRef<jbyteArray>();
  } else {
    return base::android::ToJavaByteArray(
        env,
        reinterpret_cast<const uint8*>(pickle.data()),
        pickle.size());
  }
}

jboolean XWalkContent::SetState(JNIEnv* env, jobject obj, jbyteArray state) {
  std::vector<uint8> state_vector;
  base::android::JavaByteArrayToByteVector(env, state, &state_vector);

  Pickle pickle(reinterpret_cast<const char*>(state_vector.begin()),
                state_vector.size());
  PickleIterator iterator(pickle);

  return RestoreFromPickle(&iterator, web_contents_.get());
}

static jlong Init(JNIEnv* env, jobject obj, jobject web_contents_delegate,
    jobject contents_client_bridge) {
  XWalkContent* xwalk_core_content =
    new XWalkContent(env, obj, web_contents_delegate, contents_client_bridge);
  return reinterpret_cast<intptr_t>(xwalk_core_content);
}

bool RegisterXWalkContent(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

namespace {

void ShowGeolocationPromptHelperTask(
    const JavaObjectWeakGlobalRef& java_ref,
    const GURL& origin) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> j_ref = java_ref.get(env);
  if (j_ref.obj()) {
    ScopedJavaLocalRef<jstring> j_origin(
        ConvertUTF8ToJavaString(env, origin.spec()));
    Java_XWalkContent_onGeolocationPermissionsShowPrompt(env,
                                                         j_ref.obj(),
                                                         j_origin.obj());
  }
}

void ShowGeolocationPromptHelper(const JavaObjectWeakGlobalRef& java_ref,
                                 const GURL& origin) {
  JNIEnv* env = AttachCurrentThread();
  if (java_ref.get(env).obj()) {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&ShowGeolocationPromptHelperTask,
                   java_ref,
                   origin));
  }
}
}  // anonymous namespace

void XWalkContent::ShowGeolocationPrompt(
    const GURL& requesting_frame,
    const base::Callback<void(bool)>& callback) { // NOLINT
  GURL origin = requesting_frame.GetOrigin();
  bool show_prompt = pending_geolocation_prompts_.empty();
  pending_geolocation_prompts_.push_back(OriginCallback(origin, callback));
  if (show_prompt) {
    ShowGeolocationPromptHelper(java_ref_, origin);
  }
}

// Called by Java.
void XWalkContent::InvokeGeolocationCallback(JNIEnv* env,
                                             jobject obj,
                                             jboolean value,
                                             jstring origin) {
  GURL callback_origin(base::android::ConvertJavaStringToUTF16(env, origin));
  if (callback_origin.GetOrigin() ==
      pending_geolocation_prompts_.front().first) {
    pending_geolocation_prompts_.front().second.Run(value);
    pending_geolocation_prompts_.pop_front();
    if (!pending_geolocation_prompts_.empty()) {
      ShowGeolocationPromptHelper(java_ref_,
                                  pending_geolocation_prompts_.front().first);
    }
  }
}

void XWalkContent::HideGeolocationPrompt(const GURL& origin) {
  bool removed_current_outstanding_callback = false;
  std::list<OriginCallback>::iterator it = pending_geolocation_prompts_.begin();
  while (it != pending_geolocation_prompts_.end()) {
    if ((*it).first == origin.GetOrigin()) {
      if (it == pending_geolocation_prompts_.begin()) {
        removed_current_outstanding_callback = true;
      }
      it = pending_geolocation_prompts_.erase(it);
    } else {
      ++it;
    }
  }

  if (removed_current_outstanding_callback) {
    JNIEnv* env = AttachCurrentThread();
    ScopedJavaLocalRef<jobject> j_ref = java_ref_.get(env);
    if (j_ref.obj()) {
      Java_XWalkContent_onGeolocationPermissionsHidePrompt(env, j_ref.obj());
    }
    if (!pending_geolocation_prompts_.empty()) {
      ShowGeolocationPromptHelper(java_ref_,
                            pending_geolocation_prompts_.front().first);
    }
  }
}
}  // namespace xwalk
