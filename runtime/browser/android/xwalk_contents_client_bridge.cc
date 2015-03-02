// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/callback.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "base/guid.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/desktop_notification_delegate.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/file_chooser_file_info.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/common/platform_notification_data.h"
#include "jni/XWalkContentsClientBridge_jni.h"
#include "net/cert/x509_certificate.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "url/gurl.h"
#include "ui/gfx/android/java_bitmap.h"
#include "ui/shell_dialogs/selected_file_info.h"

using base::android::AttachCurrentThread;
using base::android::ConvertJavaStringToUTF16;
using base::android::ConvertUTF8ToJavaString;
using base::android::ConvertUTF16ToJavaString;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;
using base::ScopedPtrHashMap;
using content::BrowserThread;
using content::FileChooserParams;
using content::RenderViewHost;
using content::WebContents;

namespace xwalk {

namespace {

int g_next_notification_id_ = 1;

ScopedPtrHashMap<int, content::DesktopNotificationDelegate> g_notification_map_;

}  // namespace


XWalkContentsClientBridge::XWalkContentsClientBridge(
    JNIEnv* env, jobject obj,
    content::WebContents* web_contents)
    : java_ref_(env, obj),
      icon_helper_(new XWalkIconHelper(web_contents)) {
  DCHECK(obj);
  Java_XWalkContentsClientBridge_setNativeContentsClientBridge(
      env, obj, reinterpret_cast<intptr_t>(this));
  icon_helper_->SetListener(this);
}

XWalkContentsClientBridge::~XWalkContentsClientBridge() {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;
  // Clear the weak reference from the java peer to the native object since
  // it is possible that java object lifetime can exceed the XWalkViewContents.
  Java_XWalkContentsClientBridge_setNativeContentsClientBridge(
      env, obj.obj(), 0);
}

void XWalkContentsClientBridge::AllowCertificateError(
    int cert_error,
    net::X509Certificate* cert,
    const GURL& request_url,
    const base::Callback<void(bool)>& callback, // NOLINT
    bool* cancel_request) {

  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  std::string der_string;
  if (!net::X509Certificate::GetDEREncoded(cert->os_cert_handle(),
      &der_string))
    return;
  ScopedJavaLocalRef<jbyteArray> jcert = base::android::ToJavaByteArray(
      env,
      reinterpret_cast<const uint8*>(der_string.data()),
      der_string.length());
  ScopedJavaLocalRef<jstring> jurl(ConvertUTF8ToJavaString(
      env, request_url.spec()));
  // We need to add the callback before making the call to java side,
  // as it may do a synchronous callback prior to returning.
  int request_id = pending_cert_error_callbacks_.Add(
      new CertErrorCallback(callback));
  *cancel_request = !Java_XWalkContentsClientBridge_allowCertificateError(
      env, obj.obj(), cert_error, jcert.obj(), jurl.obj(), request_id);
  // if the request is cancelled, then cancel the stored callback
  if (*cancel_request) {
    pending_cert_error_callbacks_.Remove(request_id);
  }
}

void XWalkContentsClientBridge::ProceedSslError(JNIEnv* env, jobject obj,
                                                jboolean proceed, jint id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  CertErrorCallback* callback = pending_cert_error_callbacks_.Lookup(id);
  if (!callback || callback->is_null()) {
    LOG(WARNING) << "Ignoring unexpected ssl error proceed callback";
    return;
  }
  callback->Run(proceed);
  pending_cert_error_callbacks_.Remove(id);
}

void XWalkContentsClientBridge::RunJavaScriptDialog(
    content::JavaScriptMessageType message_type,
    const GURL& origin_url,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  int callback_id = pending_js_dialog_callbacks_.Add(
      new content::JavaScriptDialogManager::DialogClosedCallback(callback));
  ScopedJavaLocalRef<jstring> jurl(
      ConvertUTF8ToJavaString(env, origin_url.spec()));
  ScopedJavaLocalRef<jstring> jmessage(
      ConvertUTF16ToJavaString(env, message_text));

  switch (message_type) {
    case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
      Java_XWalkContentsClientBridge_handleJsAlert(
          env, obj.obj(), jurl.obj(), jmessage.obj(), callback_id);
      break;
    case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
      Java_XWalkContentsClientBridge_handleJsConfirm(
          env, obj.obj(), jurl.obj(), jmessage.obj(), callback_id);
      break;
    case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT: {
      ScopedJavaLocalRef<jstring> jdefault_value(
          ConvertUTF16ToJavaString(env, default_prompt_text));
      Java_XWalkContentsClientBridge_handleJsPrompt(env,
                                                    obj.obj(),
                                                    jurl.obj(),
                                                    jmessage.obj(),
                                                    jdefault_value.obj(),
                                                    callback_id);
      break;
    }
    default:
       NOTREACHED();
  }
}

void XWalkContentsClientBridge::RunBeforeUnloadDialog(
    const GURL& origin_url,
    const base::string16& message_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  int callback_id = pending_js_dialog_callbacks_.Add(
      new content::JavaScriptDialogManager::DialogClosedCallback(callback));
  ScopedJavaLocalRef<jstring> jurl(
      ConvertUTF8ToJavaString(env, origin_url.spec()));
  ScopedJavaLocalRef<jstring> jmessage(
      ConvertUTF16ToJavaString(env, message_text));

  Java_XWalkContentsClientBridge_handleJsBeforeUnload(
      env, obj.obj(), jurl.obj(), jmessage.obj(), callback_id);
}

bool XWalkContentsClientBridge::OnReceivedHttpAuthRequest(
    const JavaRef<jobject>& handler,
    const std::string& host,
    const std::string& realm) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return false;

  ScopedJavaLocalRef<jstring> jhost = ConvertUTF8ToJavaString(env, host);
  ScopedJavaLocalRef<jstring> jrealm = ConvertUTF8ToJavaString(env, realm);
  Java_XWalkContentsClientBridge_onReceivedHttpAuthRequest(
      env, obj.obj(), handler.obj(), jhost.obj(), jrealm.obj());
  return true;
}

static void CancelNotification(
    JavaObjectWeakGlobalRef java_ref, int notification_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref.get(env);
  if (obj.is_null())
    return;

  Java_XWalkContentsClientBridge_cancelNotification(
      env, obj.obj(), notification_id);
}

void XWalkContentsClientBridge::ShowNotification(
    const content::PlatformNotificationData& notification_data,
    const SkBitmap& icon,
    scoped_ptr<content::DesktopNotificationDelegate> delegate,
    base::Closure* cancel_callback) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  ScopedJavaLocalRef<jstring> jtitle(
    ConvertUTF16ToJavaString(env, notification_data.title));
  ScopedJavaLocalRef<jstring> jbody(
    ConvertUTF16ToJavaString(env, notification_data.body));
  ScopedJavaLocalRef<jstring> jreplace_id(
    ConvertUTF16ToJavaString(env, notification_data.tag));
  ScopedJavaLocalRef<jobject> jicon;
  if (!icon.empty())
     jicon = gfx::ConvertToJavaBitmap(&icon);

  int notification_id = g_next_notification_id_++;
  g_notification_map_.set(notification_id, delegate.Pass());
  Java_XWalkContentsClientBridge_showNotification(
      env, obj.obj(), jtitle.obj(), jbody.obj(),
      jreplace_id.obj(), jicon.obj(), notification_id);

  if (cancel_callback)
    *cancel_callback = base::Bind(
        &CancelNotification, java_ref_, notification_id);
}

void XWalkContentsClientBridge::OnWebLayoutPageScaleFactorChanged(
    float page_scale_factor) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_XWalkContentsClientBridge_onWebLayoutPageScaleFactorChanged(
      env, obj.obj(), page_scale_factor);
}

void XWalkContentsClientBridge::ConfirmJsResult(JNIEnv* env,
                                                jobject,
                                                int id,
                                                jstring prompt) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  content::JavaScriptDialogManager::DialogClosedCallback* callback =
      pending_js_dialog_callbacks_.Lookup(id);
  base::string16 prompt_text;
  if (prompt) {
    prompt_text = ConvertJavaStringToUTF16(env, prompt);
  }
  if (callback)
    callback->Run(true, prompt_text);
  pending_js_dialog_callbacks_.Remove(id);
}

void XWalkContentsClientBridge::CancelJsResult(JNIEnv*, jobject, int id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  content::JavaScriptDialogManager::DialogClosedCallback* callback =
      pending_js_dialog_callbacks_.Lookup(id);
  if (callback)
    callback->Run(false, base::string16());
  pending_js_dialog_callbacks_.Remove(id);
}

void XWalkContentsClientBridge::ExitFullscreen(
    JNIEnv*, jobject, jlong j_web_contents) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  WebContents* web_contents = reinterpret_cast<WebContents*>(j_web_contents);
  if (web_contents)
    web_contents->ExitFullscreen();
}

void XWalkContentsClientBridge::NotificationDisplayed(
    JNIEnv*, jobject, jint notification_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  content::DesktopNotificationDelegate* notification_delegate =
      g_notification_map_.get(notification_id);
  if (notification_delegate)
    notification_delegate->NotificationDisplayed();
}

void XWalkContentsClientBridge::NotificationClicked(
    JNIEnv*, jobject, jint id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  scoped_ptr<content::DesktopNotificationDelegate> notification_delegate =
      g_notification_map_.take_and_erase(id);
  if (notification_delegate.get())
    notification_delegate->NotificationClick();
}

void XWalkContentsClientBridge::NotificationClosed(
    JNIEnv*, jobject, jint id, bool by_user) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  scoped_ptr<content::DesktopNotificationDelegate> notification_delegate =
      g_notification_map_.take_and_erase(id);
  if (notification_delegate.get())
    notification_delegate->NotificationClosed(by_user);
}

void XWalkContentsClientBridge::OnFilesSelected(
    JNIEnv* env, jobject, int process_id, int render_id,
    int mode, jstring filepath, jstring display_name) {
  content::RenderViewHost* rvh =
      content::RenderViewHost::FromID(process_id, render_id);
  if (!rvh)
    return;

  std::string path = base::android::ConvertJavaStringToUTF8(env, filepath);
  std::string file_name =
      base::android::ConvertJavaStringToUTF8(env, display_name);
  base::FilePath file_path = base::FilePath(path);
  content::FileChooserFileInfo file_info;
  file_info.file_path = file_path;
  if (!file_name.empty())
    file_info.display_name = file_name;
  std::vector<content::FileChooserFileInfo> files;
  files.push_back(file_info);

  rvh->FilesSelectedInChooser(
      files, static_cast<content::FileChooserParams::Mode>(mode));
}

void XWalkContentsClientBridge::OnFilesNotSelected(
    JNIEnv* env, jobject, int process_id, int render_id, int mode) {
  content::RenderViewHost* rvh =
      content::RenderViewHost::FromID(process_id, render_id);
  if (!rvh)
    return;

  std::vector<content::FileChooserFileInfo> files;

  rvh->FilesSelectedInChooser(
      files, static_cast<content::FileChooserParams::Mode>(mode));
}

void XWalkContentsClientBridge::DownloadIcon(JNIEnv* env,
                                             jobject obj,
                                             jstring url) {
  std::string url_str = base::android::ConvertJavaStringToUTF8(env, url);
  icon_helper_->DownloadIcon(GURL(url_str));
}

void XWalkContentsClientBridge::OnIconAvailable(const GURL& icon_url) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);

  ScopedJavaLocalRef<jstring> jurl(
      ConvertUTF8ToJavaString(env, icon_url.spec()));

  Java_XWalkContentsClientBridge_onIconAvailable(env, obj.obj(), jurl.obj());
}

void XWalkContentsClientBridge::OnReceivedIcon(const GURL& icon_url,
                                               const SkBitmap& bitmap) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);

  ScopedJavaLocalRef<jstring> jurl(
      ConvertUTF8ToJavaString(env, icon_url.spec()));
  ScopedJavaLocalRef<jobject> jicon = gfx::ConvertToJavaBitmap(&bitmap);

  Java_XWalkContentsClientBridge_onReceivedIcon(
      env, obj.obj(), jurl.obj(), jicon.obj());
}

bool RegisterXWalkContentsClientBridge(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
