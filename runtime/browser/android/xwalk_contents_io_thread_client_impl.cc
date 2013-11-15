// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk_contents_io_thread_client_impl.h"

#include <map>
#include <utility>

#include "intercepted_request_data_impl.h"
#include "base/android/jni_helper.h"
#include "base/android/jni_string.h"
#include "base/lazy_instance.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "jni/XWalkContentsIoThreadClient_jni.h"
#include "net/url_request/url_request.h"
#include "url/gurl.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;
using base::LazyInstance;
using content::BrowserThread;
using content::RenderViewHost;
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
    RenderViewHostToIoThreadClientType;

static pair<int, int> GetRenderViewHostIdPair(RenderViewHost* rvh) {
  return pair<int, int>(rvh->GetProcess()->GetID(), rvh->GetRoutingID());
}

// RvhToIoThreadClientMap -----------------------------------------------------
class RvhToIoThreadClientMap {
 public:
  static RvhToIoThreadClientMap* GetInstance();
  void Set(pair<int, int> rvh_id, const IoThreadClientData& client);
  bool Get(pair<int, int> rvh_id, IoThreadClientData* client);
  void Erase(pair<int, int> rvh_id);

 private:
  static LazyInstance<RvhToIoThreadClientMap> g_instance_;
  base::Lock map_lock_;
  RenderViewHostToIoThreadClientType rvh_to_io_thread_client_;
};

// static
LazyInstance<RvhToIoThreadClientMap> RvhToIoThreadClientMap::g_instance_ =
    LAZY_INSTANCE_INITIALIZER;

// static
RvhToIoThreadClientMap* RvhToIoThreadClientMap::GetInstance() {
  return g_instance_.Pointer();
}

void RvhToIoThreadClientMap::Set(pair<int, int> rvh_id,
                                 const IoThreadClientData& client) {
  base::AutoLock lock(map_lock_);
  rvh_to_io_thread_client_[rvh_id] = client;
}

bool RvhToIoThreadClientMap::Get(
    pair<int, int> rvh_id, IoThreadClientData* client) {
  base::AutoLock lock(map_lock_);
  RenderViewHostToIoThreadClientType::iterator iterator =
      rvh_to_io_thread_client_.find(rvh_id);
  if (iterator == rvh_to_io_thread_client_.end())
    return false;

  *client = iterator->second;
  return true;
}

void RvhToIoThreadClientMap::Erase(pair<int, int> rvh_id) {
  base::AutoLock lock(map_lock_);
  rvh_to_io_thread_client_.erase(rvh_id);
}

// ClientMapEntryUpdater ------------------------------------------------------

class ClientMapEntryUpdater : public content::WebContentsObserver {
 public:
  ClientMapEntryUpdater(JNIEnv* env, WebContents* web_contents,
                        jobject jdelegate);

  virtual void RenderViewCreated(RenderViewHost* render_view_host) OVERRIDE;
  virtual void RenderViewForInterstitialPageCreated(
      RenderViewHost* render_view_host) OVERRIDE;
  virtual void RenderViewDeleted(RenderViewHost* render_view_host) OVERRIDE;
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

  if (web_contents->GetRenderViewHost())
    RenderViewCreated(web_contents->GetRenderViewHost());
}

void ClientMapEntryUpdater::RenderViewCreated(RenderViewHost* rvh) {
  IoThreadClientData client_data;
  client_data.io_thread_client = jdelegate_;
  client_data.pending_association = false;
  RvhToIoThreadClientMap::GetInstance()->Set(
      GetRenderViewHostIdPair(rvh), client_data);
}

void ClientMapEntryUpdater::RenderViewForInterstitialPageCreated(
    RenderViewHost* rvh) {
  RenderViewCreated(rvh);
}

void ClientMapEntryUpdater::RenderViewDeleted(RenderViewHost* rvh) {
  RvhToIoThreadClientMap::GetInstance()->Erase(GetRenderViewHostIdPair(rvh));
}

void ClientMapEntryUpdater::WebContentsDestroyed(WebContents* web_contents) {
  if (web_contents->GetRenderViewHost())
    RenderViewDeleted(web_contents->GetRenderViewHost());
  delete this;
}

} // namespace

// XWalkContentsIoThreadClientImpl -----------------------------------------------

// static
scoped_ptr<XWalkContentsIoThreadClient>
XWalkContentsIoThreadClient::FromID(int render_process_id, int render_view_id) {
  pair<int, int> rvh_id(render_process_id, render_view_id);
  IoThreadClientData client_data;
  if (!RvhToIoThreadClientMap::GetInstance()->Get(rvh_id, &client_data))
    return scoped_ptr<XWalkContentsIoThreadClient>();

  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> java_delegate =
      client_data.io_thread_client.get(env);
  DCHECK(!client_data.pending_association || java_delegate.is_null());
  return scoped_ptr<XWalkContentsIoThreadClient>(new XWalkContentsIoThreadClientImpl(
      client_data.pending_association, java_delegate));
}

// static
void XWalkContentsIoThreadClientImpl::RegisterPendingContents(
    WebContents* web_contents) {
  IoThreadClientData client_data;
  client_data.pending_association = true;
  RvhToIoThreadClientMap::GetInstance()->Set(
      GetRenderViewHostIdPair(web_contents->GetRenderViewHost()), client_data);
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

void XWalkContentsIoThreadClientImpl::NewLoginRequest(const std::string& realm,
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

} // namespace xwalk
