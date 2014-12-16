// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_content.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_delegate.h"

#include <string>
#include <vector>

#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/file_chooser_file_info.h"
#include "content/public/common/file_chooser_params.h"
#include "jni/XWalkWebContentsDelegate_jni.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/runtime_file_select_helper.h"
#include "xwalk/runtime/browser/runtime_javascript_dialog_manager.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF16ToJavaString;
using base::android::ScopedJavaLocalRef;
using content::FileChooserParams;
using content::WebContents;

namespace xwalk {

XWalkWebContentsDelegate::XWalkWebContentsDelegate(
    JNIEnv* env,
    jobject obj)
    : WebContentsDelegateAndroid(env, obj) {
}

XWalkWebContentsDelegate::~XWalkWebContentsDelegate() {
}

void XWalkWebContentsDelegate::AddNewContents(
    content::WebContents* source,
    content::WebContents* new_contents,
    WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos,
    bool user_gesture,
    bool* was_blocked) {
  JNIEnv* env = AttachCurrentThread();

  bool is_dialog = disposition == NEW_POPUP;
  ScopedJavaLocalRef<jobject> java_delegate = GetJavaDelegate(env);
  bool create_popup = false;

  if (java_delegate.obj()) {
    create_popup = Java_XWalkWebContentsDelegate_addNewContents(env,
        java_delegate.obj(), is_dialog, user_gesture);
  }

  if (create_popup) {
    XWalkContent::FromWebContents(source)->SetPendingWebContentsForPopup(
        make_scoped_ptr(new_contents));
    new_contents->WasHidden();
  } else {
    base::MessageLoop::current()->DeleteSoon(FROM_HERE, new_contents);
  }

  if (was_blocked) {
    *was_blocked = !create_popup;
  }
}

void XWalkWebContentsDelegate::CloseContents(content::WebContents* source) {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> java_delegate = GetJavaDelegate(env);
  if (java_delegate.obj()) {
    Java_XWalkWebContentsDelegate_closeContents(env, java_delegate.obj());
  }
}

void XWalkWebContentsDelegate::ActivateContents(content::WebContents* source) {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> java_delegate = GetJavaDelegate(env);
  if (java_delegate.obj()) {
    Java_XWalkWebContentsDelegate_activateContents(env, java_delegate.obj());
  }
}

void XWalkWebContentsDelegate::UpdatePreferredSize(
    content::WebContents* contents,
    const gfx::Size& pref_size) {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> java_delegate = GetJavaDelegate(env);
  if (java_delegate.obj()) {
    Java_XWalkWebContentsDelegate_updatePreferredSize(env, java_delegate.obj(),
        pref_size.width(), pref_size.height());
  }
}

void XWalkWebContentsDelegate::RunFileChooser(
    content::WebContents* web_contents,
    const content::FileChooserParams& params) {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> java_delegate = GetJavaDelegate(env);
  if (!java_delegate.obj())
    return;

  if (params.mode == FileChooserParams::Save) {
    // Save not supported, so cancel it.
    web_contents->GetRenderViewHost()->FilesSelectedInChooser(
         std::vector<content::FileChooserFileInfo>(),
         params.mode);
    return;
  }
  int mode = static_cast<int>(params.mode);
  jboolean overridden =
      Java_XWalkWebContentsDelegate_shouldOverrideRunFileChooser(env,
          java_delegate.obj(),
          web_contents->GetRenderProcessHost()->GetID(),
          web_contents->GetRenderViewHost()->GetRoutingID(),
          mode,
          ConvertUTF16ToJavaString(env,
              JoinString(params.accept_types, ',')).obj(),
          params.capture);
  if (overridden == JNI_FALSE)
    RuntimeFileSelectHelper::RunFileChooser(web_contents, params);
}

content::JavaScriptDialogManager*
XWalkWebContentsDelegate::GetJavaScriptDialogManager() {
  if (!javascript_dialog_manager_.get()) {
    javascript_dialog_manager_.reset(new RuntimeJavaScriptDialogManager);
  }
  return javascript_dialog_manager_.get();
}

void XWalkWebContentsDelegate::RequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  XWalkMediaCaptureDevicesDispatcher::RunRequestMediaAccessPermission(
      web_contents, request, callback);
}

void XWalkWebContentsDelegate::RendererUnresponsive(WebContents* source) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = GetJavaDelegate(env);
  if (obj.is_null())
    return;
  Java_XWalkWebContentsDelegate_rendererUnresponsive(env, obj.obj());
}

void XWalkWebContentsDelegate::RendererResponsive(WebContents* source) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = GetJavaDelegate(env);
  if (obj.is_null())
    return;
  Java_XWalkWebContentsDelegate_rendererResponsive(env, obj.obj());
}

void XWalkWebContentsDelegate::HandleKeyboardEvent(
    content::WebContents* source,
    const content::NativeWebKeyboardEvent& event) {
  jobject key_event = event.os_event;
  if (!key_event)
    return;
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = GetJavaDelegate(env);
  if (obj.is_null())
    return;
  Java_XWalkWebContentsDelegate_handleKeyboardEvent(env, obj.obj(), key_event);
}


void XWalkWebContentsDelegate::ToggleFullscreenModeForTab(
    content::WebContents* web_contents,
    bool enter_fullscreen) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = GetJavaDelegate(env);
  if (obj.is_null())
    return;
  Java_XWalkWebContentsDelegate_toggleFullscreen(
      env, obj.obj(), enter_fullscreen);
}

bool XWalkWebContentsDelegate::IsFullscreenForTabOrPending(
    const content::WebContents* web_contents) const {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = GetJavaDelegate(env);
  if (obj.is_null())
    return false;
  return Java_XWalkWebContentsDelegate_isFullscreen(env, obj.obj());
}

bool XWalkWebContentsDelegate::ShouldCreateWebContents(
    content::WebContents* web_contents,
    int route_id,
    WindowContainerType window_container_type,
    const base::string16& frame_name,
    const GURL& target_url,
    const std::string& partition_id,
    content::SessionStorageNamespace* session_storage_namespace) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> java_delegate = GetJavaDelegate(env);

  if (java_delegate.obj()) {
    ScopedJavaLocalRef<jstring> url_buffer =
        base::android::ConvertUTF8ToJavaString(env, target_url.spec());
    return Java_XWalkWebContentsDelegate_shouldOpenWithDefaultBrowser(env,
        java_delegate.obj(), url_buffer.obj()) == JNI_FALSE;
  }

  // As multiple windows mode has not been implemented yet, return false
  // to make sure new WebContents won't be created.
  return false;
}

bool RegisterXWalkWebContentsDelegate(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
