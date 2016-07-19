// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_settings.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/command_line.h"
#include "components/user_prefs/user_prefs.h"
#include "content/browser/android/java/jni_helper.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/web_preferences.h"
#include "jni/XWalkSettingsInternal_jni.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/common/xwalk_content_client.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/runtime/browser/android/renderer_host/xwalk_render_view_host_ext.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"

using base::android::CheckException;
using base::android::ConvertJavaStringToUTF16;
using base::android::ConvertUTF16ToJavaString;
using base::android::ConvertUTF8ToJavaString;
using base::android::GetClass;
using base::android::ScopedJavaLocalRef;
using content::GetFieldID;

namespace xwalk {

struct XWalkSettings::FieldIds {
  // Note on speed. One may think that an approach that reads field values via
  // JNI is ineffective and should not be used. Please keep in mind that in the
  // legacy WebView the whole Sync method took <1ms on Xoom, and no one is
  // expected to modify settings in performance-critical code.
  FieldIds() = default;

  explicit FieldIds(JNIEnv* env) {
    const char* kStringClassName = "Ljava/lang/String;";

    // FIXME: we should be using a new GetFieldIDFromClassName() with caching.
    ScopedJavaLocalRef<jclass> clazz(
        GetClass(env, "org/xwalk/core/internal/XWalkSettingsInternal"));
    allow_scripts_to_close_windows =
        GetFieldID(env, clazz, "mAllowScriptsToCloseWindows", "Z");
    load_images_automatically =
        GetFieldID(env, clazz, "mLoadsImagesAutomatically", "Z");
    images_enabled =
        GetFieldID(env, clazz, "mImagesEnabled", "Z");
    java_script_enabled =
        GetFieldID(env, clazz, "mJavaScriptEnabled", "Z");
    allow_universal_access_from_file_urls =
        GetFieldID(env, clazz, "mAllowUniversalAccessFromFileURLs", "Z");
    allow_file_access_from_file_urls =
        GetFieldID(env, clazz, "mAllowFileAccessFromFileURLs", "Z");
    java_script_can_open_windows_automatically =
        GetFieldID(env, clazz, "mJavaScriptCanOpenWindowsAutomatically", "Z");
    support_multiple_windows =
        GetFieldID(env, clazz, "mSupportMultipleWindows", "Z");
    dom_storage_enabled =
        GetFieldID(env, clazz, "mDomStorageEnabled", "Z");
    database_enabled =
        GetFieldID(env, clazz, "mDatabaseEnabled", "Z");
    use_wide_viewport =
        GetFieldID(env, clazz, "mUseWideViewport", "Z");
    media_playback_requires_user_gesture =
        GetFieldID(env, clazz, "mMediaPlaybackRequiresUserGesture", "Z");
    default_video_poster_url =
        GetFieldID(env, clazz, "mDefaultVideoPosterURL", kStringClassName);
    text_size_percent =
        GetFieldID(env, clazz, "mTextSizePercent", "I");
    default_font_size =
        GetFieldID(env, clazz, "mDefaultFontSize", "I");
    default_fixed_font_size =
        GetFieldID(env, clazz, "mDefaultFixedFontSize", "I");
    spatial_navigation_enabled =
        GetFieldID(env, clazz, "mSpatialNavigationEnabled", "Z");
    quirks_mode_enabled =
        GetFieldID(env, clazz, "mQuirksModeEnabled", "Z");
    initialize_at_minimum_page_scale =
        GetFieldID(env, clazz, "mLoadWithOverviewMode", "Z");
  }

  // Field ids
  jfieldID allow_scripts_to_close_windows;
  jfieldID load_images_automatically;
  jfieldID images_enabled;
  jfieldID java_script_enabled;
  jfieldID allow_universal_access_from_file_urls;
  jfieldID allow_file_access_from_file_urls;
  jfieldID java_script_can_open_windows_automatically;
  jfieldID support_multiple_windows;
  jfieldID dom_storage_enabled;
  jfieldID database_enabled;
  jfieldID use_wide_viewport;
  jfieldID media_playback_requires_user_gesture;
  jfieldID default_video_poster_url;
  jfieldID text_size_percent;
  jfieldID default_font_size;
  jfieldID default_fixed_font_size;
  jfieldID spatial_navigation_enabled;
  jfieldID quirks_mode_enabled;
  jfieldID initialize_at_minimum_page_scale;
};

XWalkSettings::XWalkSettings(JNIEnv* env,
                             jobject obj,
                             content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      xwalk_settings_(env, obj) {
}

XWalkSettings::~XWalkSettings() {
    JNIEnv* env = base::android::AttachCurrentThread();
    ScopedJavaLocalRef<jobject> scoped_obj = xwalk_settings_.get(env);
    jobject obj = scoped_obj.obj();
    if (!obj) return;
    Java_XWalkSettingsInternal_nativeXWalkSettingsGone(
        env, obj, reinterpret_cast<intptr_t>(this));
}

void XWalkSettings::Destroy(JNIEnv* env, jobject obj) {
  delete this;
}

XWalkRenderViewHostExt* XWalkSettings::GetXWalkRenderViewHostExt() {
  if (!web_contents()) return NULL;
  XWalkContent* contents = XWalkContent::FromWebContents(web_contents());
  if (!contents) return NULL;
  return contents->render_view_host_ext();
}

void XWalkSettings::UpdateEverything() {
  JNIEnv* env = base::android::AttachCurrentThread();
  CHECK(env);
  ScopedJavaLocalRef<jobject> scoped_obj = xwalk_settings_.get(env);
  jobject obj = scoped_obj.obj();
  if (!obj) return;

  Java_XWalkSettingsInternal_updateEverything(env, obj);
}

void XWalkSettings::UpdateEverythingLocked(JNIEnv* env, jobject obj) {
  UpdateInitialPageScale(env, obj);
  UpdateWebkitPreferences(env, obj);
  UpdateUserAgent(env, obj);
  UpdateFormDataPreferences(env, obj);
}

void XWalkSettings::UpdateUserAgent(JNIEnv* env, jobject obj) {
  if (!web_contents()) return;

  ScopedJavaLocalRef<jstring> str =
      Java_XWalkSettingsInternal_getUserAgentLocked(env, obj);
  bool ua_overidden = str.obj() != NULL;

  if (ua_overidden) {
    std::string override = base::android::ConvertJavaStringToUTF8(str);
    web_contents()->SetUserAgentOverride(override);
  }

  const content::NavigationController& controller =
      web_contents()->GetController();
  for (int i = 0; i < controller.GetEntryCount(); ++i)
    controller.GetEntryAtIndex(i)->SetIsOverridingUserAgent(ua_overidden);
}

void XWalkSettings::UpdateWebkitPreferences(JNIEnv* env, jobject obj) {
  if (!web_contents()) return;
  XWalkRenderViewHostExt* render_view_host_ext = GetXWalkRenderViewHostExt();
  if (!render_view_host_ext) return;

  if (!field_ids_)
    field_ids_.reset(new FieldIds(env));

  content::RenderViewHost* render_view_host =
      web_contents()->GetRenderViewHost();
  if (!render_view_host) return;
  content::WebPreferences prefs = render_view_host->GetWebkitPreferences();

  prefs.allow_scripts_to_close_windows =
      env->GetBooleanField(obj, field_ids_->allow_scripts_to_close_windows);

  // Blink's LoadsImagesAutomatically and ImagesEnabled must be
  // set cris-cross to Android's. See
  // https://code.google.com/p/chromium/issues/detail?id=224317#c26
  prefs.images_enabled =
      env->GetBooleanField(obj, field_ids_->load_images_automatically);
  prefs.loads_images_automatically =
      env->GetBooleanField(obj, field_ids_->images_enabled);

  prefs.javascript_enabled =
      env->GetBooleanField(obj, field_ids_->java_script_enabled);

  prefs.allow_universal_access_from_file_urls = env->GetBooleanField(
      obj, field_ids_->allow_universal_access_from_file_urls);

  prefs.allow_file_access_from_file_urls = env->GetBooleanField(
      obj, field_ids_->allow_file_access_from_file_urls);

  prefs.javascript_can_open_windows_automatically = env->GetBooleanField(
      obj, field_ids_->java_script_can_open_windows_automatically);

  prefs.supports_multiple_windows = env->GetBooleanField(
      obj, field_ids_->support_multiple_windows);

  prefs.application_cache_enabled =
      Java_XWalkSettingsInternal_getAppCacheEnabled(env, obj);

  prefs.local_storage_enabled = env->GetBooleanField(
      obj, field_ids_->dom_storage_enabled);

  prefs.databases_enabled = env->GetBooleanField(
      obj, field_ids_->database_enabled);

  prefs.initialize_at_minimum_page_scale =
      env->GetBooleanField(obj, field_ids_->initialize_at_minimum_page_scale);
  prefs.double_tap_to_zoom_enabled = prefs.use_wide_viewport =
      env->GetBooleanField(obj, field_ids_->use_wide_viewport);

  prefs.user_gesture_required_for_media_playback = env->GetBooleanField(
      obj, field_ids_->media_playback_requires_user_gesture);

  prefs.password_echo_enabled =
      Java_XWalkSettingsInternal_getPasswordEchoEnabledLocked(env, obj);

  prefs.double_tap_to_zoom_enabled =
      Java_XWalkSettingsInternal_supportsDoubleTapZoomLocked(env, obj);

  prefs.spatial_navigation_enabled = env->GetBooleanField(
      obj, field_ids_->spatial_navigation_enabled);

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  prefs.allow_running_insecure_content =
      command_line->HasSwitch(switches::kAllowRunningInsecureContent);
  prefs.allow_displaying_insecure_content =
      !command_line->HasSwitch(switches::kNoDisplayingInsecureContent);

  ScopedJavaLocalRef<jstring> str;
  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->default_video_poster_url)));
  prefs.default_video_poster_url = str.obj() ?
      GURL(ConvertJavaStringToUTF8(str)) : GURL();

  int text_size_percent = env->GetIntField(obj, field_ids_->text_size_percent);

  prefs.text_autosizing_enabled =
      Java_XWalkSettingsInternal_getTextAutosizingEnabledLocked(env, obj);
  if (prefs.text_autosizing_enabled) {
    prefs.font_scale_factor = text_size_percent / 100.0f;
    prefs.force_enable_zoom = text_size_percent >= 130;
    // Use the default zoom factor value when Text Autosizer is turned on.
    render_view_host_ext->SetTextZoomFactor(1);
  } else {
    prefs.force_enable_zoom = false;
    render_view_host_ext->SetTextZoomFactor(text_size_percent / 100.0f);
  }

  int font_size = env->GetIntField(obj, field_ids_->default_font_size);
  prefs.default_font_size = font_size;

  int fixed_font_size = env->GetIntField(obj,
      field_ids_->default_fixed_font_size);
  prefs.default_fixed_font_size = fixed_font_size;

  bool support_quirks = env->GetBooleanField(
      obj, field_ids_->quirks_mode_enabled);
  prefs.viewport_meta_non_user_scalable_quirk = support_quirks;
  prefs.clobber_user_agent_initial_scale_quirk = support_quirks;

  prefs.wide_viewport_quirk = true;
  prefs.viewport_meta_enabled = true;

  render_view_host->UpdateWebkitPreferences(prefs);
}

void XWalkSettings::UpdateFormDataPreferences(JNIEnv* env, jobject obj) {
  if (!web_contents()) return;
  XWalkContent* content = XWalkContent::FromWebContents(web_contents());
  if (!content) return;
  content->SetSaveFormData(
      Java_XWalkSettingsInternal_getSaveFormDataLocked(env, obj));
}

void XWalkSettings::RenderViewCreated(
    content::RenderViewHost* render_view_host) {
  // A single WebContents can normally have 0 to many RenderViewHost instances
  // associated with it.
  // This is important since there is only one RenderViewHostExt instance per
  // WebContents (and not one RVHExt per RVH, as you might expect) and updating
  // settings via RVHExt only ever updates the 'current' RVH.
  // In android_webview we don't swap out the RVH on cross-site navigations, so
  // we shouldn't have to deal with the multiple RVH per WebContents case. That
  // in turn means that the newly created RVH is always the 'current' RVH
  // (since we only ever go from 0 to 1 RVH instances) and hence the DCHECK.
  DCHECK(web_contents()->GetRenderViewHost() == render_view_host);

  UpdateEverything();
}

void XWalkSettings::UpdateAcceptLanguages(JNIEnv* env, jobject obj) {
  PrefService* pref_service = GetPrefs();
  if (!pref_service) return;
  pref_service->SetString(
      "intl.accept_languages",
      base::android::ConvertJavaStringToUTF8(
          Java_XWalkSettingsInternal_getAcceptLanguagesLocked(env, obj)));
}

PrefService* XWalkSettings::GetPrefs() {
  return user_prefs::UserPrefs::Get(XWalkBrowserContext::GetDefault());
}

static jlong Init(JNIEnv* env,
                 const JavaParamRef<jobject>& obj,
                 const JavaParamRef<jobject>& web_contents) {
  content::WebContents* contents = content::WebContents::FromJavaWebContents(
      web_contents);
  XWalkSettings* settings = new XWalkSettings(env, obj, contents);
  return reinterpret_cast<intptr_t>(settings);
}

static ScopedJavaLocalRef<jstring>
GetDefaultUserAgent(JNIEnv* env, const JavaParamRef<jclass>& clazz) {
  return base::android::ConvertUTF8ToJavaString(env, GetUserAgent());
}

void XWalkSettings::UpdateInitialPageScale(JNIEnv* env, jobject obj) {
  if (!web_contents()) return;
  XWalkRenderViewHostExt* render_view_host_ext = GetXWalkRenderViewHostExt();
  if (!render_view_host_ext) return;

  float initial_page_scale_percent =
      Java_XWalkSettingsInternal_getInitialPageScalePercentLocked(env, obj);
  if (initial_page_scale_percent == 0) {
    render_view_host_ext->SetInitialPageScale(-1);
    return;
  }
  float dip_scale = static_cast<float>(
      Java_XWalkSettingsInternal_getDIPScaleLocked(env, obj));
  render_view_host_ext->SetInitialPageScale(
      initial_page_scale_percent / dip_scale / 100.0f);
}

void XWalkSettings::ResetScrollAndScaleState(JNIEnv* env, jobject obj) {
  XWalkRenderViewHostExt* rvhe = GetXWalkRenderViewHostExt();
  if (!rvhe) return;
  rvhe->ResetScrollAndScaleState();
}

bool RegisterXWalkSettings(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
