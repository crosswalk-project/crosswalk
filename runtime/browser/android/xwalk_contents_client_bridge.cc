// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"

#include <string>

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/callback.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/show_desktop_notification_params.h"
#include "jni/XWalkContentsClientBridge_jni.h"
#include "net/cert/x509_certificate.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "url/gurl.h"
#include "ui/gfx/android/java_bitmap.h"

using base::android::AttachCurrentThread;
using base::android::ConvertJavaStringToUTF16;
using base::android::ConvertUTF8ToJavaString;
using base::android::ConvertUTF16ToJavaString;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;
using content::BrowserThread;
using content::RenderViewHost;
using content::WebContents;

namespace xwalk {

namespace {

void RunUpdateNotificationIconOnUIThread(
    int notification_id,
    int process_id,
    int route_id,
    const SkBitmap& icon) {
  XWalkContentsClientBridgeBase* bridge =
      XWalkContentsClientBridgeBase::FromRenderViewID(process_id, route_id);
  if (bridge)
    bridge->UpdateNotificationIcon(notification_id, icon);
}

}  // namespace

XWalkContentsClientBridge::XWalkContentsClientBridge(JNIEnv* env, jobject obj)
    : java_ref_(env, obj) {
  DCHECK(obj);
  Java_XWalkContentsClientBridge_setNativeContentsClientBridge(
      env, obj, reinterpret_cast<jint>(this));
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
    const base::Callback<void(bool)>& callback,
    bool* cancel_request) {

  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  std::string der_string;
  net::X509Certificate::GetDEREncoded(cert->os_cert_handle(), &der_string);
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

void XWalkContentsClientBridge::OnNotificationIconDownloaded(
    int id,
    int http_status_code,
    const GURL& icon_url,
    const std::vector<SkBitmap>& bitmaps,
    const std::vector<gfx::Size>& original_bitmap_sizes) {
  if (bitmaps.empty() && http_status_code == 404) {
    LOG(WARNING) << "Failed to download notification icon from "
                 << icon_url.spec();
  } else {
    NotificationDownloadRequestIdMap::iterator iter =
        downloading_icon_notifications_.find(id);
    if (iter == downloading_icon_notifications_.end() ||
        iter->second.size() != 3) {
      return;
    }
    int notification_id = iter->second[0];
    int process_id = iter->second[1];
    int route_id = iter->second[2];
    // This will lead to a second call of ShowNotification for the
    // same notification id to update the icon. On Android, when
    // the notification which is already shown is fired again, it will
    // silently update the content only.
    BrowserThread::PostTask(
        BrowserThread::UI,
        FROM_HERE,
        base::Bind(&RunUpdateNotificationIconOnUIThread,
                   notification_id,
                   process_id,
                   route_id,
                   bitmaps[0]));
  }
  downloading_icon_notifications_.erase(id);
}

void XWalkContentsClientBridge::UpdateNotificationIcon(
    int notification_id, const SkBitmap& icon) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  ScopedJavaLocalRef<jobject> jicon = gfx::ConvertToJavaBitmap(&icon);
  Java_XWalkContentsClientBridge_updateNotificationIcon(
      env, obj.obj(), notification_id, jicon.obj());
}

void XWalkContentsClientBridge::ShowNotification(
    const content::ShowDesktopNotificationHostMsgParams& params,
    bool worker,
    int process_id,
    int route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  ScopedJavaLocalRef<jstring> jtitle(
    ConvertUTF16ToJavaString(env, params.title));
  ScopedJavaLocalRef<jstring> jbody(
    ConvertUTF16ToJavaString(env, params.body));
  ScopedJavaLocalRef<jstring> jreplace_id(
    ConvertUTF16ToJavaString(env, params.replace_id));

  Java_XWalkContentsClientBridge_showNotification(
      env, obj.obj(), jtitle.obj(), jbody.obj(),
      jreplace_id.obj(), params.notification_id,
      process_id, route_id);

  if (params.icon_url.is_valid()) {
    RenderViewHost* rvh = RenderViewHost::FromID(process_id, route_id);
    if (rvh) {
      WebContents* web_contents = WebContents::FromRenderViewHost(rvh);
      if (web_contents) {
        int download_request_id = web_contents->DownloadImage(
            params.icon_url,
            false,
            0,
            base::Bind(
                &XWalkContentsClientBridge::OnNotificationIconDownloaded,
                base::Unretained(this)));
        std::vector<int> ids;
        ids.push_back(params.notification_id);
        ids.push_back(process_id);
        ids.push_back(route_id);
        downloading_icon_notifications_[download_request_id] = ids;
      }
    }
  }
}

void XWalkContentsClientBridge::CancelNotification(
    int notification_id,
    int process_id,
    int route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;

  Java_XWalkContentsClientBridge_cancelNotification(
      env, obj.obj(), notification_id,
      process_id, route_id);
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
    JNIEnv*, jobject, jint j_web_contents) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  WebContents* web_contents = reinterpret_cast<WebContents*>(j_web_contents);
  if (web_contents) {
    RenderViewHost* rvh = web_contents->GetRenderViewHost();
    if (rvh)
      rvh->ExitFullscreen();
  }
}

void XWalkContentsClientBridge::NotificationDisplayed(
    JNIEnv*, jobject, int id, int process_id, int route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  RenderViewHost* rvh = RenderViewHost::FromID(
      process_id, route_id);
  if (!rvh)
    return;
  rvh->DesktopNotificationPostDisplay(id);
}

void XWalkContentsClientBridge::NotificationError(
    JNIEnv* env, jobject, int id, jstring error,
    int process_id, int route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  RenderViewHost* rvh = RenderViewHost::FromID(
      process_id, route_id);
  if (!rvh)
    return;
  base::string16 error_text;
  if (error)
    error_text = ConvertJavaStringToUTF16(env, error);
  rvh->DesktopNotificationPostError(id, error_text);
}

void XWalkContentsClientBridge::NotificationClicked(
    JNIEnv*, jobject, int id, int process_id, int route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  RenderViewHost* rvh = RenderViewHost::FromID(
      process_id, route_id);
  if (!rvh)
    return;
  rvh->DesktopNotificationPostClick(id);
}

void XWalkContentsClientBridge::NotificationClosed(
    JNIEnv*, jobject, int id, bool by_user,
    int process_id, int route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  RenderViewHost* rvh = RenderViewHost::FromID(
      process_id, route_id);
  if (!rvh)
    return;
  rvh->DesktopNotificationPostClose(id, by_user);
}

bool RegisterXWalkContentsClientBridge(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
