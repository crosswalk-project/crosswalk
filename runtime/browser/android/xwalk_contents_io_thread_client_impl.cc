// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client_impl.h"

#include <map>
#include <string>
#include <utility>

#include "base/android/jni_string.h"
#include "base/android/jni_weak_ref.h"
#include "base/lazy_instance.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "jni/XWalkContentsIoThreadClient_jni.h"
#include "net/url_request/url_request.h"
#include "url/gurl.h"
#include "xwalk/runtime/browser/android/intercepted_request_data_impl.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;
using base::LazyInstance;
using content::BrowserThread;
using content::RenderFrameHost;
using content::WebContents;
using std::map;
using std::pair;

namespace xwalk {

namespace {

struct IoThreadClientData {
  bool pending_association;
  JavaObjectWeakGlobalRef io_thread_client;

  IoThreadClientData();
};

IoThreadClientData::IoThreadClientData() : pending_association(false) {}

typedef map<pair<int, int>, IoThreadClientData>
    RenderFrameHostToIoThreadClientType;

static pair<int, int> GetRenderFrameHostIdPair(RenderFrameHost* rfh) {
  return pair<int, int>(rfh->GetProcess()->GetID(), rfh->GetRoutingID());
}

// RfhToIoThreadClientMap -----------------------------------------------------
class RfhToIoThreadClientMap {
 public:
  static RfhToIoThreadClientMap* GetInstance();
  void Set(pair<int, int> rfh_id, const IoThreadClientData& client);
  bool Get(pair<int, int> rfh_id, IoThreadClientData* client);
  void Erase(pair<int, int> rfh_id);

 private:
  static LazyInstance<RfhToIoThreadClientMap> g_instance_;
  base::Lock map_lock_;
  RenderFrameHostToIoThreadClientType rfh_to_io_thread_client_;
};

// static
LazyInstance<RfhToIoThreadClientMap> RfhToIoThreadClientMap::g_instance_ =
    LAZY_INSTANCE_INITIALIZER;

// static
RfhToIoThreadClientMap* RfhToIoThreadClientMap::GetInstance() {
  return g_instance_.Pointer();
}

void RfhToIoThreadClientMap::Set(pair<int, int> rfh_id,
                                 const IoThreadClientData& client) {
  base::AutoLock lock(map_lock_);
  rfh_to_io_thread_client_[rfh_id] = client;
}

bool RfhToIoThreadClientMap::Get(
    pair<int, int> rfh_id, IoThreadClientData* client) {
  base::AutoLock lock(map_lock_);
  RenderFrameHostToIoThreadClientType::iterator iterator =
      rfh_to_io_thread_client_.find(rfh_id);
  if (iterator == rfh_to_io_thread_client_.end())
    return false;

  *client = iterator->second;
  return true;
}

void RfhToIoThreadClientMap::Erase(pair<int, int> rfh_id) {
  base::AutoLock lock(map_lock_);
  rfh_to_io_thread_client_.erase(rfh_id);
}

// ClientMapEntryUpdater ------------------------------------------------------

class ClientMapEntryUpdater : public content::WebContentsObserver {
 public:
  ClientMapEntryUpdater(JNIEnv* env, WebContents* web_contents,
                        jobject jdelegate);

  virtual void RenderFrameCreated(RenderFrameHost* render_frame_host) OVERRIDE;
  virtual void RenderFrameDeleted(RenderFrameHost* render_frame_host) OVERRIDE;
  virtual void WebContentsDestroyed(WebContents* web_contents) OVERRIDE;

 private:
  JavaObjectWeakGlobalRef jdelegate_;
};

ClientMapEntryUpdater::ClientMapEntryUpdater(JNIEnv* env,
                                             WebContents* web_contents,
                                             jobject jdelegate)
    : content::WebContentsObserver(web_contents),
      jdelegate_(env, jdelegate) {
  DCHECK(web_contents);
  DCHECK(jdelegate);

  if (web_contents->GetMainFrame())
    RenderFrameCreated(web_contents->GetMainFrame());
}

void ClientMapEntryUpdater::RenderFrameCreated(RenderFrameHost* rfh) {
  IoThreadClientData client_data;
  client_data.io_thread_client = jdelegate_;
  client_data.pending_association = false;
  RfhToIoThreadClientMap::GetInstance()->Set(
      GetRenderFrameHostIdPair(rfh), client_data);
}


void ClientMapEntryUpdater::RenderFrameDeleted(RenderFrameHost* rfh) {
  RfhToIoThreadClientMap::GetInstance()->Erase(GetRenderFrameHostIdPair(rfh));
}

void ClientMapEntryUpdater::WebContentsDestroyed(WebContents* web_contents) {
  delete this;
}

}  // namespace

// XWalkContentsIoThreadClientImpl -------------------------------------------

// static
scoped_ptr<XWalkContentsIoThreadClient>
XWalkContentsIoThreadClient::FromID(int render_process_id,
                                    int render_frame_id) {
  pair<int, int> rfh_id(render_process_id, render_frame_id);
  IoThreadClientData client_data;
  if (!RfhToIoThreadClientMap::GetInstance()->Get(rfh_id, &client_data))
    return scoped_ptr<XWalkContentsIoThreadClient>();

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> java_delegate =
      client_data.io_thread_client.get(env);
  DCHECK(!client_data.pending_association || java_delegate.is_null());
  return scoped_ptr<XWalkContentsIoThreadClient>(
      new XWalkContentsIoThreadClientImpl(
          client_data.pending_association, java_delegate));
}

// static
void XWalkContentsIoThreadClient::SubFrameCreated(int render_process_id,
                                                  int parent_render_frame_id,
                                                  int child_render_frame_id) {
  pair<int, int> parent_rfh_id(render_process_id, parent_render_frame_id);
  pair<int, int> child_rfh_id(render_process_id, child_render_frame_id);
  IoThreadClientData client_data;
  if (!RfhToIoThreadClientMap::GetInstance()->Get(parent_rfh_id,
                                                  &client_data)) {
    NOTREACHED();
    return;
  }

  RfhToIoThreadClientMap::GetInstance()->Set(child_rfh_id, client_data);
}


// static
void XWalkContentsIoThreadClientImpl::RegisterPendingContents(
    WebContents* web_contents) {
  IoThreadClientData client_data;
  client_data.pending_association = true;
  RfhToIoThreadClientMap::GetInstance()->Set(
      GetRenderFrameHostIdPair(web_contents->GetMainFrame()), client_data);
}

// static
void XWalkContentsIoThreadClientImpl::Associate(
    WebContents* web_contents,
    const JavaRef<jobject>& jclient) {
  JNIEnv* env = AttachCurrentThread();
  // The ClientMapEntryUpdater lifespan is tied to the WebContents.
  new ClientMapEntryUpdater(env, web_contents, jclient.obj());
}

XWalkContentsIoThreadClientImpl::XWalkContentsIoThreadClientImpl(
    bool pending_association,
    const JavaRef<jobject>& obj)
  : pending_association_(pending_association),
    java_object_(obj) {
}

XWalkContentsIoThreadClientImpl::~XWalkContentsIoThreadClientImpl() {
  // explict, out-of-line destructor.
}

bool XWalkContentsIoThreadClientImpl::PendingAssociation() const {
  return pending_association_;
}

XWalkContentsIoThreadClient::CacheMode
XWalkContentsIoThreadClientImpl::GetCacheMode() const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (java_object_.is_null())
    return XWalkContentsIoThreadClient::LOAD_DEFAULT;

  JNIEnv* env = AttachCurrentThread();
  return static_cast<XWalkContentsIoThreadClient::CacheMode>(
      Java_XWalkContentsIoThreadClient_getCacheMode(
          env, java_object_.obj()));
}

scoped_ptr<InterceptedRequestData>
XWalkContentsIoThreadClientImpl::ShouldInterceptRequest(
    const GURL& location,
    const net::URLRequest* request) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (java_object_.is_null())
    return scoped_ptr<InterceptedRequestData>();
  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);
  bool is_main_frame = info &&
      info->GetResourceType() == ResourceType::MAIN_FRAME;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> jstring_url =
      ConvertUTF8ToJavaString(env, location.spec());
  ScopedJavaLocalRef<jobject> ret =
      Java_XWalkContentsIoThreadClient_shouldInterceptRequest(
          env, java_object_.obj(), jstring_url.obj(), is_main_frame);
  if (ret.is_null())
    return scoped_ptr<InterceptedRequestData>();
  return scoped_ptr<InterceptedRequestData>(
      new InterceptedRequestDataImpl(ret));
}

bool XWalkContentsIoThreadClientImpl::ShouldBlockContentUrls() const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (java_object_.is_null())
    return false;

  JNIEnv* env = AttachCurrentThread();
  return Java_XWalkContentsIoThreadClient_shouldBlockContentUrls(
      env, java_object_.obj());
}

bool XWalkContentsIoThreadClientImpl::ShouldBlockFileUrls() const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (java_object_.is_null())
    return false;

  JNIEnv* env = AttachCurrentThread();
  return Java_XWalkContentsIoThreadClient_shouldBlockFileUrls(
      env, java_object_.obj());
}

bool XWalkContentsIoThreadClientImpl::ShouldBlockNetworkLoads() const {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (java_object_.is_null())
    return false;

  JNIEnv* env = AttachCurrentThread();
  return Java_XWalkContentsIoThreadClient_shouldBlockNetworkLoads(
      env, java_object_.obj());
}

void XWalkContentsIoThreadClientImpl::NewDownload(
    const GURL& url,
    const std::string& user_agent,
    const std::string& content_disposition,
    const std::string& mime_type,
    int64 content_length) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (java_object_.is_null())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> jstring_url =
      ConvertUTF8ToJavaString(env, url.spec());
  ScopedJavaLocalRef<jstring> jstring_user_agent =
      ConvertUTF8ToJavaString(env, user_agent);
  ScopedJavaLocalRef<jstring> jstring_content_disposition =
      ConvertUTF8ToJavaString(env, content_disposition);
  ScopedJavaLocalRef<jstring> jstring_mime_type =
      ConvertUTF8ToJavaString(env, mime_type);

  Java_XWalkContentsIoThreadClient_onDownloadStart(
      env,
      java_object_.obj(),
      jstring_url.obj(),
      jstring_user_agent.obj(),
      jstring_content_disposition.obj(),
      jstring_mime_type.obj(),
      content_length);
}

void XWalkContentsIoThreadClientImpl::NewLoginRequest(
    const std::string& realm,
    const std::string& account,
    const std::string& args) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (java_object_.is_null())
    return;

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> jrealm = ConvertUTF8ToJavaString(env, realm);
  ScopedJavaLocalRef<jstring> jargs = ConvertUTF8ToJavaString(env, args);

  ScopedJavaLocalRef<jstring> jaccount;
  if (!account.empty())
    jaccount = ConvertUTF8ToJavaString(env, account);

  Java_XWalkContentsIoThreadClient_newLoginRequest(
      env, java_object_.obj(), jrealm.obj(), jaccount.obj(), jargs.obj());
}

bool RegisterXWalkContentsIoThreadClientImpl(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
