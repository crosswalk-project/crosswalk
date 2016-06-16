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
#include "base/android/locale_utils.h"
#include "base/base_paths_android.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/memory/ptr_util.h"
#include "base/path_service.h"
#include "base/pickle.h"
#include "base/strings/string_number_conversions.h"
#include "components/navigation_interception/intercept_navigation_delegate.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cert_store.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/ssl_status.h"
#include "content/public/common/url_constants.h"
#include "ui/gfx/geometry/rect_f.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/runtime/browser/android/net_disk_cache_remover.h"
#include "xwalk/runtime/browser/android/state_serializer.h"
#include "xwalk/runtime/browser/android/xwalk_autofill_client_android.h"
#include "xwalk/runtime/browser/android/xwalk_content_lifecycle_notifier.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge_base.h"
#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client_impl.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_delegate.h"
#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate_android.h"
#include "xwalk/runtime/browser/xwalk_autofill_manager.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "jni/XWalkContent_jni.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::ConvertUTF16ToJavaString;
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
      return nullptr;
    XWalkContentUserData* data = reinterpret_cast<XWalkContentUserData*>(
      web_contents->GetUserData(kXWalkContentUserDataKey));
    return data ? data->content_ : nullptr;
  }

 private:
  XWalkContent* content_;
};

// FIXME(wang16): Remove following methods after deprecated fields
// are not supported any more.
void PrintManifestDeprecationWarning(std::string field) {
  LOG(WARNING) << "\"" << field << "\" is deprecated for Crosswalk. "
      << "Please follow "
      << "https://www.crosswalk-project.org/#documentation/manifest.";
}

bool ManifestHasPath(const xwalk::application::Manifest& manifest,
                     const std::string& path,
                     const std::string& deprecated_path) {
  if (manifest.HasPath(path))
    return true;
  if (manifest.HasPath(deprecated_path)) {
    PrintManifestDeprecationWarning(deprecated_path);
    return true;
  }
  return false;
}

bool ManifestGetString(const xwalk::application::Manifest& manifest,
                       const std::string& path,
                       const std::string& deprecated_path,
                       std::string* out_value) {
  if (manifest.GetString(path, out_value))
    return true;
  if (manifest.GetString(deprecated_path, out_value)) {
    PrintManifestDeprecationWarning(deprecated_path);
    return true;
  }
  return false;
}

}  // namespace

// static
XWalkContent* XWalkContent::FromID(int render_process_id,
                                   int render_view_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  content::RenderViewHost* rvh =
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

XWalkContent::XWalkContent(std::unique_ptr<content::WebContents> web_contents)
    : web_contents_(std::move(web_contents)) {
  xwalk_autofill_manager_.reset(new XWalkAutofillManager(web_contents_.get()));
  XWalkContentLifecycleNotifier::OnXWalkViewCreated();
}

void XWalkContent::SetXWalkAutofillClient(jobject client) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) return;
  Java_XWalkContent_setXWalkAutofillClient(env, obj.obj(), client);
}

void XWalkContent::SetSaveFormData(bool enabled) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  xwalk_autofill_manager_->InitAutofillIfNecessary(enabled);
  // We need to check for the existence, since autofill_manager_delegate
  // may not be created when the setting is false.
  if (auto client =
      XWalkAutofillClientAndroid::FromWebContents(web_contents_.get()))
    client->SetSaveFormData(enabled);
}

XWalkContent::~XWalkContent() {
  XWalkContentLifecycleNotifier::OnXWalkViewDestroyed();
}

void XWalkContent::SetJavaPeers(JNIEnv* env,
                                jobject obj,
                                jobject xwalk_content,
                                jobject web_contents_delegate,
                                jobject contents_client_bridge,
                                jobject io_thread_client,
                                jobject intercept_navigation_delegate) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  java_ref_ = JavaObjectWeakGlobalRef(env, xwalk_content);

  web_contents_delegate_.reset(new XWalkWebContentsDelegate(
      env, web_contents_delegate));
  contents_client_bridge_.reset(new XWalkContentsClientBridge(
      env, contents_client_bridge, web_contents_.get()));

  web_contents_->SetUserData(
      kXWalkContentUserDataKey, new XWalkContentUserData(this));

  XWalkContentsIoThreadClientImpl::RegisterPendingContents(web_contents_.get());

  // XWalk does not use disambiguation popup for multiple targets.
  content::RendererPreferences* prefs =
      web_contents_->GetMutableRendererPrefs();
  prefs->tap_multiple_targets_strategy =
      content::TAP_MULTIPLE_TARGETS_STRATEGY_NONE;

  XWalkContentsClientBridgeBase::Associate(web_contents_.get(),
      contents_client_bridge_.get());
  XWalkContentsIoThreadClientImpl::Associate(web_contents_.get(),
      ScopedJavaLocalRef<jobject>(env, io_thread_client));
  int render_process_id = web_contents_->GetRenderProcessHost()->GetID();
  int render_frame_id = web_contents_->GetRoutingID();
  RuntimeResourceDispatcherHostDelegateAndroid::OnIoThreadClientReady(
      render_process_id, render_frame_id);
  InterceptNavigationDelegate::Associate(web_contents_.get(),
      base::WrapUnique(new InterceptNavigationDelegate(
          env, intercept_navigation_delegate)));
  web_contents_->SetDelegate(web_contents_delegate_.get());

  render_view_host_ext_.reset(new XWalkRenderViewHostExt(web_contents_.get()));
}

base::android::ScopedJavaLocalRef<jobject>
XWalkContent::GetWebContents(JNIEnv* env, jobject obj) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DCHECK(web_contents_);
  if (!web_contents_)
    return base::android::ScopedJavaLocalRef<jobject>();
  return web_contents_->GetJavaWebContents();
}

void XWalkContent::SetPendingWebContentsForPopup(
    std::unique_ptr<content::WebContents> pending) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (pending_contents_.get()) {
    // TODO(benm): Support holding multiple pop up window requests.
    LOG(WARNING) << "Blocking popup window creation as an outstanding "
                 << "popup window is still pending.";
    base::MessageLoop::current()->DeleteSoon(FROM_HERE, pending.release());
    return;
  }
  pending_contents_.reset(new XWalkContent(std::move(pending)));
}

jlong XWalkContent::ReleasePopupXWalkContent(JNIEnv* env, jobject obj) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  return reinterpret_cast<intptr_t>(pending_contents_.release());
}

void XWalkContent::ClearCache(
    JNIEnv* env,
    jobject obj,
    jboolean include_disk_files) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  render_view_host_ext_->ClearCache();

  if (include_disk_files)
    RemoveHttpDiskCache(web_contents_->GetRenderProcessHost(), std::string());
}

void XWalkContent::ClearCacheForSingleFile(
    JNIEnv* env,
    jobject obj,
    jstring url) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  std::string key = base::android::ConvertJavaStringToUTF8(env, url);

  if (!key.empty())
    RemoveHttpDiskCache(web_contents_->GetRenderProcessHost(), key);
}

ScopedJavaLocalRef<jstring> XWalkContent::DevToolsAgentId(JNIEnv* env,
                                                          jobject obj) {
  scoped_refptr<content::DevToolsAgentHost> agent_host(
      content::DevToolsAgentHost::GetOrCreateFor(web_contents_.get()));
  return base::android::ConvertUTF8ToJavaString(env, agent_host->GetId());
}

void XWalkContent::Destroy(JNIEnv* env, jobject obj) {
  delete this;
}

void XWalkContent::RequestNewHitTestDataAt(JNIEnv* env,
                                           jobject obj,
                                           jfloat x,
                                           jfloat y,
                                           jfloat touch_major) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  gfx::PointF touch_center(x, y);
  gfx::SizeF touch_area(touch_major, touch_major);
  render_view_host_ext_->RequestNewHitTestDataAt(touch_center, touch_area);
}

void XWalkContent::UpdateLastHitTestData(JNIEnv* env, jobject obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!render_view_host_ext_->HasNewHitTestData()) return;

  const XWalkHitTestData& data = render_view_host_ext_->GetLastHitTestData();
  render_view_host_ext_->MarkHitTestDataRead();

  // Make sure to null the java object if data is empty/invalid
  ScopedJavaLocalRef<jstring> extra_data_for_type;
  if (data.extra_data_for_type.length())
    extra_data_for_type = ConvertUTF8ToJavaString(
      env, data.extra_data_for_type);

  ScopedJavaLocalRef<jstring> href;
  if (data.href.length())
    href = ConvertUTF16ToJavaString(env, data.href);

  ScopedJavaLocalRef<jstring> anchor_text;
  if (data.anchor_text.length())
    anchor_text = ConvertUTF16ToJavaString(env, data.anchor_text);

  ScopedJavaLocalRef<jstring> img_src;
  if (data.img_src.is_valid())
    img_src = ConvertUTF8ToJavaString(env, data.img_src.spec());
  Java_XWalkContent_updateHitTestData(env,
                                      obj,
                                      data.type,
                                      extra_data_for_type.obj(),
                                      href.obj(),
                                      anchor_text.obj(),
                                      img_src.obj());
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

  std::unique_ptr<base::Value> manifest_value = base::JSONReader::Read(json_input);
  if (!manifest_value || !manifest_value->IsType(base::Value::TYPE_DICTIONARY))
      return false;

  xwalk::application::Manifest manifest(
      base::WrapUnique(
          static_cast<base::DictionaryValue*>(manifest_value.release())));

  std::string url;
  if (manifest.GetString(keys::kStartURLKey, &url)) {
    std::string scheme = GURL(url).scheme();
    if (scheme.empty())
      url = path_str + url;
  } else if (manifest.GetString(keys::kLaunchLocalPathKey, &url)) {
    PrintManifestDeprecationWarning(keys::kLaunchLocalPathKey);
    // According to original proposal for "app:launch:local_path", the "http"
    // and "https" schemes are supported. So |url| should do nothing when it
    // already has "http" or "https" scheme.
    std::string scheme = GURL(url).scheme();
    if (scheme != url::kHttpScheme && scheme != url::kHttpsScheme)
      url = path_str + url;
  } else if (manifest.GetString(keys::kLaunchWebURLKey, &url)) {
    PrintManifestDeprecationWarning(keys::kLaunchWebURLKey);
  } else {
    NOTIMPLEMENTED();
  }

  std::string match_patterns;
  const base::ListValue* xwalk_hosts = NULL;
  if (manifest.GetList(
          xwalk::application_manifest_keys::kXWalkHostsKey, &xwalk_hosts)) {
      base::JSONWriter::Write(*xwalk_hosts, &match_patterns);
  }
  render_view_host_ext_->SetOriginAccessWhitelist(url, match_patterns);

  std::string csp;
  ManifestGetString(manifest, keys::kCSPKey, keys::kDeprecatedCSPKey, &csp);
  XWalkBrowserContext* browser_context =
      XWalkRunner::GetInstance()->browser_context();
  CHECK(browser_context);
  browser_context->SetCSPString(csp);

  ScopedJavaLocalRef<jstring> url_buffer =
      base::android::ConvertUTF8ToJavaString(env, url);

  if (manifest.HasPath(kDisplay)) {
    std::string display_string;
    if (manifest.GetString(kDisplay, &display_string)) {
      // TODO(David): update the handling process of the display strings
      // including fullscreen etc.
      bool display_as_fullscreen =
          base::LowerCaseEqualsASCII(display_string, "fullscreen");
      Java_XWalkContent_onGetFullscreenFlagFromManifest(
          env, obj, display_as_fullscreen ? JNI_TRUE : JNI_FALSE);
    }
  }

  // Check whether need to display launch screen. (Read from manifest.json)
  if (ManifestHasPath(manifest,
                      keys::kXWalkLaunchScreen,
                      keys::kLaunchScreen)) {
    std::string ready_when;
    // Get the value of 'ready_when' from manifest.json
    ManifestGetString(manifest,
                      keys::kXWalkLaunchScreenReadyWhen,
                      keys::kLaunchScreenReadyWhen,
                      &ready_when);
    ScopedJavaLocalRef<jstring> ready_when_buffer =
        base::android::ConvertUTF8ToJavaString(env, ready_when);

    // Get the value of 'image_border'
    // 1. When 'launch_screen.[orientation]' was defined, but no 'image_border'
    //    The value of 'image_border' will be set as 'empty'.
    // 2. Otherwise, there is no 'launch_screen.[orientation]' defined,
    //    The value of 'image_border' will be empty.
    const char empty[] = "empty";
    std::string image_border_default;
    ManifestGetString(manifest,
                      keys::kXWalkLaunchScreenImageBorderDefault,
                      keys::kLaunchScreenImageBorderDefault,
                      &image_border_default);
    if (image_border_default.empty() &&
        ManifestHasPath(manifest,
                        keys::kXWalkLaunchScreenDefault,
                        keys::kLaunchScreenDefault)) {
      image_border_default = empty;
    }

    std::string image_border_landscape;
    ManifestGetString(manifest,
                      keys::kXWalkLaunchScreenImageBorderLandscape,
                      keys::kLaunchScreenImageBorderLandscape,
                      &image_border_landscape);
    if (image_border_landscape.empty() &&
        ManifestHasPath(manifest,
                        keys::kXWalkLaunchScreenLandscape,
                        keys::kLaunchScreenLandscape)) {
      image_border_landscape = empty;
    }

    std::string image_border_portrait;
    ManifestGetString(manifest,
                      keys::kXWalkLaunchScreenImageBorderPortrait,
                      keys::kLaunchScreenImageBorderPortrait,
                      &image_border_portrait);
    if (image_border_portrait.empty() &&
        ManifestHasPath(manifest,
                        keys::kXWalkLaunchScreenPortrait,
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
  std::string view_background_color;
  ManifestGetString(manifest,
                    keys::kXWalkViewBackgroundColor,
                    keys::kViewBackgroundColor,
                    &view_background_color);

  if (view_background_color.empty())
    return true;
  unsigned int view_background_color_int = 0;
  if (!base::HexStringToUInt(view_background_color.substr(1),
      &view_background_color_int)) {
    LOG(ERROR) << "Background color format error! Valid background color"
               "should be(Alpha Red Green Blue): #ff01abcd";
    return false;
  }
  Java_XWalkContent_setBackgroundColor(env, obj, view_background_color_int);
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

  base::Pickle pickle;
  if (!WriteToPickle(*web_contents_, &pickle)) {
    return ScopedJavaLocalRef<jbyteArray>();
  } else {
    return base::android::ToJavaByteArray(
        env,
        reinterpret_cast<const uint8_t*>(pickle.data()),
        pickle.size());
  }
}

jboolean XWalkContent::SetState(JNIEnv* env, jobject obj, jbyteArray state) {
  std::vector<uint8_t> state_vector;
  base::android::JavaByteArrayToByteVector(env, state, &state_vector);

  base::Pickle pickle(reinterpret_cast<const char*>(&state_vector[0]),
                state_vector.size());
  base::PickleIterator iterator(pickle);

  return RestoreFromPickle(&iterator, web_contents_.get());
}

static jlong Init(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  std::unique_ptr<WebContents> web_contents(content::WebContents::Create(
      content::WebContents::CreateParams(
          XWalkRunner::GetInstance()->browser_context())));
  return reinterpret_cast<intptr_t>(new XWalkContent(std::move(web_contents)));
}

bool RegisterXWalkContent(JNIEnv* env) {
  return RegisterNativesImpl(env);
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

// Called by Java.
void XWalkContent::SetBackgroundColor(JNIEnv* env, jobject obj, jint color) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  render_view_host_ext_->SetBackgroundColor(color);
}

void XWalkContent::SetOriginAccessWhitelist(JNIEnv* env, jobject obj,
                                            jstring url,
                                            jstring match_patterns) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  render_view_host_ext_->SetOriginAccessWhitelist(
      base::android::ConvertJavaStringToUTF8(env, url),
      base::android::ConvertJavaStringToUTF8(env, match_patterns));
}

base::android::ScopedJavaLocalRef<jbyteArray> XWalkContent::GetCertificate(
    JNIEnv* env,
    const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  content::NavigationEntry* entry =
      web_contents_->GetController().GetLastCommittedEntry();
  if (!entry)
    return ScopedJavaLocalRef<jbyteArray>();
  // Get the certificate
  int cert_id = entry->GetSSL().cert_id;
  scoped_refptr<net::X509Certificate> cert;
  bool ok = content::CertStore::GetInstance()->RetrieveCert(cert_id, &cert);
  if (!ok)
    return ScopedJavaLocalRef<jbyteArray>();

  // Convert the certificate and return it
  std::string der_string;
  net::X509Certificate::GetDEREncoded(cert->os_cert_handle(), &der_string);
  return base::android::ToJavaByteArray(
      env, reinterpret_cast<const uint8_t*>(der_string.data()),
      der_string.length());
}

FindHelper* XWalkContent::GetFindHelper() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!find_helper_.get()) {
    find_helper_.reset(new FindHelper(web_contents_.get()));
    find_helper_->SetListener(this);
  }
  return find_helper_.get();
}

void XWalkContent::FindAllAsync(JNIEnv* env,
                                const JavaParamRef<jobject>& obj,
                                const JavaParamRef<jstring>& search_string) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  GetFindHelper()->FindAllAsync(ConvertJavaStringToUTF16(env, search_string));
}

void XWalkContent::FindNext(JNIEnv* env,
                            const JavaParamRef<jobject>& obj,
                            jboolean forward) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  GetFindHelper()->FindNext(forward);
}

void XWalkContent::ClearMatches(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  GetFindHelper()->ClearMatches();
}

void XWalkContent::OnFindResultReceived(int active_ordinal,
                                        int match_count,
                                        bool finished) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  Java_XWalkContent_onFindResultReceived(
      env, obj.obj(), active_ordinal, match_count, finished);
}

}  // namespace xwalk
