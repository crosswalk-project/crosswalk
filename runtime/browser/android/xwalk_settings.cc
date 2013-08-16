// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_settings.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_client.h"
#include "jni/XWalkSettings_jni.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/user_agent/user_agent.h"
#include "xwalk/runtime/browser/android/renderer_host/xwalk_render_view_host_ext.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"

using base::android::CheckException;
using base::android::ConvertJavaStringToUTF16;
using base::android::ConvertUTF16ToJavaString;
using base::android::ConvertUTF8ToJavaString;
using base::android::GetClass;
using base::android::GetFieldID;
using base::android::GetMethodIDFromClassName;
using base::android::ScopedJavaLocalRef;

namespace xwalk {

struct XWalkSettings::FieldIds {
  // Note on speed. One may think that an approach that reads field values via
  // JNI is ineffective and should not be used. Please keep in mind that in the
  // legacy WebView the whole Sync method took <1ms on Xoom, and no one is
  // expected to modify settings in performance-critical code.
  FieldIds() { }

  FieldIds(JNIEnv* env) {
    const char* kStringClassName = "Ljava/lang/String;";

    // FIXME: we should be using a new GetFieldIDFromClassName() with caching.
    ScopedJavaLocalRef<jclass> clazz(
        GetClass(env, "org/xwalk/core/XWalkSettings"));
    text_size_percent = GetFieldID(env, clazz, "mTextSizePercent", "I");
    standard_fond_family =
        GetFieldID(env, clazz, "mStandardFontFamily", kStringClassName);
    fixed_font_family =
        GetFieldID(env, clazz, "mFixedFontFamily", kStringClassName);
    sans_serif_font_family =
        GetFieldID(env, clazz, "mSansSerifFontFamily", kStringClassName);
    serif_font_family =
        GetFieldID(env, clazz, "mSerifFontFamily", kStringClassName);
    cursive_font_family =
        GetFieldID(env, clazz, "mCursiveFontFamily", kStringClassName);
    fantasy_font_family =
        GetFieldID(env, clazz, "mFantasyFontFamily", kStringClassName);
    default_text_encoding =
        GetFieldID(env, clazz, "mDefaultTextEncoding", kStringClassName);
    user_agent =
        GetFieldID(env, clazz, "mUserAgent", kStringClassName);
    minimum_font_size = GetFieldID(env, clazz, "mMinimumFontSize", "I");
    minimum_logical_font_size =
        GetFieldID(env, clazz, "mMinimumLogicalFontSize", "I");
    default_font_size = GetFieldID(env, clazz, "mDefaultFontSize", "I");
    default_fixed_font_size =
        GetFieldID(env, clazz, "mDefaultFixedFontSize", "I");
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
    load_with_overview_mode =
        GetFieldID(env, clazz, "mLoadWithOverviewMode", "Z");
    media_playback_requires_user_gesture =
        GetFieldID(env, clazz, "mMediaPlaybackRequiresUserGesture", "Z");
    default_video_poster_url =
        GetFieldID(env, clazz, "mDefaultVideoPosterURL", kStringClassName);
    support_deprecated_target_density_dpi =
        GetFieldID(env, clazz, "mSupportDeprecatedTargetDensityDPI", "Z");
    dip_scale =
        GetFieldID(env, clazz, "mDIPScale", "D");
    initial_page_scale_percent =
        GetFieldID(env, clazz, "mInitialPageScalePercent", "F");
  }

  // Field ids
  jfieldID text_size_percent;
  jfieldID standard_fond_family;
  jfieldID fixed_font_family;
  jfieldID sans_serif_font_family;
  jfieldID serif_font_family;
  jfieldID cursive_font_family;
  jfieldID fantasy_font_family;
  jfieldID default_text_encoding;
  jfieldID user_agent;
  jfieldID minimum_font_size;
  jfieldID minimum_logical_font_size;
  jfieldID default_font_size;
  jfieldID default_fixed_font_size;
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
  jfieldID load_with_overview_mode;
  jfieldID media_playback_requires_user_gesture;
  jfieldID default_video_poster_url;
  jfieldID support_deprecated_target_density_dpi;
  jfieldID dip_scale;
  jfieldID initial_page_scale_percent;
};

XWalkSettings::XWalkSettings(JNIEnv* env, jobject obj)
    : xwalk_settings_(env, obj) {
}

XWalkSettings::~XWalkSettings() {
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

void XWalkSettings::ResetScrollAndScaleState(JNIEnv* env, jobject obj) {
  XWalkRenderViewHostExt* rvhe = GetXWalkRenderViewHostExt();
  if (!rvhe) return;
  rvhe->ResetScrollAndScaleState();
}

void XWalkSettings::SetWebContents(JNIEnv* env, jobject obj, jint jweb_contents) {
  content::WebContents* web_contents =
      reinterpret_cast<content::WebContents*>(jweb_contents);
  Observe(web_contents);

  UpdateEverything(env, obj);
}

void XWalkSettings::UpdateEverything() {
  JNIEnv* env = base::android::AttachCurrentThread();
  CHECK(env);
  ScopedJavaLocalRef<jobject> scoped_obj = xwalk_settings_.get(env);
  jobject obj = scoped_obj.obj();
  if (!obj) return;
  UpdateEverything(env, obj);
}

void XWalkSettings::UpdateEverything(JNIEnv* env, jobject obj) {
  UpdateInitialPageScale(env, obj);
  UpdateWebkitPreferences(env, obj);
  UpdateUserAgent(env, obj);
  ResetScrollAndScaleState(env, obj);
  UpdatePreferredSizeMode();
}

void XWalkSettings::UpdateUserAgent(JNIEnv* env, jobject obj) {
  if (!web_contents()) return;

  if (!field_ids_)
    field_ids_.reset(new FieldIds(env));

  ScopedJavaLocalRef<jstring> str(env, static_cast<jstring>(
      env->GetObjectField(obj, field_ids_->user_agent)));
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
  WebPreferences prefs = render_view_host->GetWebkitPreferences();

  prefs.text_autosizing_enabled =
      Java_XWalkSettings_getTextAutosizingEnabled(env, obj);

  int text_size_percent = env->GetIntField(obj, field_ids_->text_size_percent);
  if (prefs.text_autosizing_enabled) {
    prefs.font_scale_factor = text_size_percent / 100.0f;
    prefs.force_enable_zoom = text_size_percent >= 130;
    // Use the default zoom level value when Text Autosizer is turned on.
    render_view_host_ext->SetTextZoomLevel(0);
  } else {
    prefs.force_enable_zoom = false;
    render_view_host_ext->SetTextZoomLevel(webkit_glue::ZoomFactorToZoomLevel(
        text_size_percent / 100.0f));
  }

  ScopedJavaLocalRef<jstring> str(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->standard_fond_family)));
  prefs.standard_font_family_map[webkit_glue::kCommonScript] =
      ConvertJavaStringToUTF16(str);

  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->fixed_font_family)));
  prefs.fixed_font_family_map[webkit_glue::kCommonScript] =
      ConvertJavaStringToUTF16(str);

  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->sans_serif_font_family)));
  prefs.sans_serif_font_family_map[webkit_glue::kCommonScript] =
      ConvertJavaStringToUTF16(str);

  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->serif_font_family)));
  prefs.serif_font_family_map[webkit_glue::kCommonScript] =
      ConvertJavaStringToUTF16(str);

  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->cursive_font_family)));
  prefs.cursive_font_family_map[webkit_glue::kCommonScript] =
      ConvertJavaStringToUTF16(str);

  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->fantasy_font_family)));
  prefs.fantasy_font_family_map[webkit_glue::kCommonScript] =
      ConvertJavaStringToUTF16(str);

  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->default_text_encoding)));
  prefs.default_encoding = ConvertJavaStringToUTF8(str);

  prefs.minimum_font_size =
      env->GetIntField(obj, field_ids_->minimum_font_size);

  prefs.minimum_logical_font_size =
      env->GetIntField(obj, field_ids_->minimum_logical_font_size);

  prefs.default_font_size =
      env->GetIntField(obj, field_ids_->default_font_size);

  prefs.default_fixed_font_size =
      env->GetIntField(obj, field_ids_->default_fixed_font_size);

  prefs.loads_images_automatically =
      env->GetBooleanField(obj, field_ids_->load_images_automatically);

  prefs.images_enabled =
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

  prefs.plugins_enabled = !Java_XWalkSettings_getPluginsDisabled(env, obj);

  prefs.application_cache_enabled =
      Java_XWalkSettings_getAppCacheEnabled(env, obj);

  prefs.local_storage_enabled = env->GetBooleanField(
      obj, field_ids_->dom_storage_enabled);

  prefs.databases_enabled = env->GetBooleanField(
      obj, field_ids_->database_enabled);

  prefs.double_tap_to_zoom_enabled = prefs.use_wide_viewport =
      env->GetBooleanField(obj, field_ids_->use_wide_viewport);

  prefs.initialize_at_minimum_page_scale = env->GetBooleanField(
      obj, field_ids_->load_with_overview_mode);

  prefs.user_gesture_required_for_media_playback = env->GetBooleanField(
      obj, field_ids_->media_playback_requires_user_gesture);

  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->default_video_poster_url)));
  prefs.default_video_poster_url = str.obj() ?
      GURL(ConvertJavaStringToUTF8(str)) : GURL();

  prefs.support_deprecated_target_density_dpi = env->GetBooleanField(
      obj, field_ids_->support_deprecated_target_density_dpi);

  render_view_host->UpdateWebkitPreferences(prefs);
}

void XWalkSettings::UpdateInitialPageScale(JNIEnv* env, jobject obj) {
  XWalkRenderViewHostExt* rvhe = GetXWalkRenderViewHostExt();
  if (!rvhe) return;

  if (!field_ids_)
    field_ids_.reset(new FieldIds(env));

  float initial_page_scale_percent =
      env->GetFloatField(obj, field_ids_->initial_page_scale_percent);
  if (initial_page_scale_percent == 0) {
    rvhe->SetInitialPageScale(-1);
  } else {
    float dip_scale = static_cast<float>(
        env->GetDoubleField(obj, field_ids_->dip_scale));
    rvhe->SetInitialPageScale(initial_page_scale_percent / dip_scale / 100.0f);
  }
}

void XWalkSettings::UpdatePreferredSizeMode() {
  if (web_contents()->GetRenderViewHost()) {
    web_contents()->GetRenderViewHost()->EnablePreferredSizeMode();
  }
}

void XWalkSettings::RenderViewCreated(content::RenderViewHost* render_view_host) {
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

static jint Init(JNIEnv* env,
                 jobject obj,
                 jint web_contents) {
  XWalkSettings* settings = new XWalkSettings(env, obj);
  settings->SetWebContents(env, obj, web_contents);
  return reinterpret_cast<jint>(settings);
}

static jstring GetDefaultUserAgent(JNIEnv* env, jclass clazz) {
  return base::android::ConvertUTF8ToJavaString(
      env, content::GetUserAgent(GURL())).Release();
}

bool RegisterXWalkSettings(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
