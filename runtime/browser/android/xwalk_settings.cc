// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_settings.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "content/browser/renderer_host/java/jni_helper.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_client.h"
#include "jni/XWalkSettings_jni.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/common/webpreferences.h"
#include "webkit/common/user_agent/user_agent.h"
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
  FieldIds() { }

  explicit FieldIds(JNIEnv* env) {
    const char* kStringClassName = "Ljava/lang/String;";

    // FIXME: we should be using a new GetFieldIDFromClassName() with caching.
    ScopedJavaLocalRef<jclass> clazz(
        GetClass(env, "org/xwalk/core/XWalkSettings"));
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
  }

  // Field ids
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

void XWalkSettings::SetWebContents(JNIEnv* env,
                                   jobject obj,
                                   jint jweb_contents) {
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
  UpdateWebkitPreferences(env, obj);
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

  prefs.application_cache_enabled =
      Java_XWalkSettings_getAppCacheEnabled(env, obj);

  prefs.local_storage_enabled = env->GetBooleanField(
      obj, field_ids_->dom_storage_enabled);

  prefs.databases_enabled = env->GetBooleanField(
      obj, field_ids_->database_enabled);

  prefs.double_tap_to_zoom_enabled = prefs.use_wide_viewport =
      env->GetBooleanField(obj, field_ids_->use_wide_viewport);

  prefs.user_gesture_required_for_media_playback = env->GetBooleanField(
      obj, field_ids_->media_playback_requires_user_gesture);

  ScopedJavaLocalRef<jstring> str;
  str.Reset(
      env, static_cast<jstring>(
          env->GetObjectField(obj, field_ids_->default_video_poster_url)));
  prefs.default_video_poster_url = str.obj() ?
      GURL(ConvertJavaStringToUTF8(str)) : GURL();

  render_view_host->UpdateWebkitPreferences(prefs);
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

static jint Init(JNIEnv* env,
                 jobject obj,
                 jint web_contents) {
  XWalkSettings* settings = new XWalkSettings(env, obj);
  settings->SetWebContents(env, obj, web_contents);
  return reinterpret_cast<jint>(settings);
}

bool RegisterXWalkSettings(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
