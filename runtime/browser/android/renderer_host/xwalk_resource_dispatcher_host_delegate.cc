// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/renderer_host/xwalk_resource_dispatcher_host_delegate.h"

#include <string>

#include "xwalk/runtime/browser/android/xwalk_contents_io_thread_client.h"
#include "xwalk/runtime/browser/android/xwalk_login_delegate.h"
#include "xwalk/runtime/browser/android/net/url_constants.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "components/auto_login_parser/auto_login_parser.h"
#include "components/navigation_interception/intercept_navigation_delegate.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_controller.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/resource_throttle.h"
#include "content/public/common/url_constants.h"
#include "net/base/load_flags.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"

using xwalk::XWalkContentsIoThreadClient;
using content::BrowserThread;
using navigation_interception::InterceptNavigationDelegate;

namespace {

base::LazyInstance<xwalk::XWalkResourceDispatcherHostDelegate>
    g_webview_resource_dispatcher_host_delegate = LAZY_INSTANCE_INITIALIZER;

void SetCacheControlFlag(
    net::URLRequest* request, int flag) {
  const int all_cache_control_flags = net::LOAD_BYPASS_CACHE |
      net::LOAD_VALIDATE_CACHE |
      net::LOAD_PREFERRING_CACHE |
      net::LOAD_ONLY_FROM_CACHE;
  DCHECK((flag & all_cache_control_flags) == flag);
  int load_flags = request->load_flags();
  load_flags &= ~all_cache_control_flags;
  load_flags |= flag;
  request->set_load_flags(load_flags);
}

}  // namespace

namespace xwalk {

// Calls through the IoThreadClient to check the embedders settings to determine
// if the request should be cancelled. There may not always be an IoThreadClient
// available for the |child_id|, |route_id| pair (in the case of newly created
// pop up windows, for example) and in that case the request and the client
// callbacks will be deferred the request until a client is ready.
class IoThreadClientThrottle : public content::ResourceThrottle {
 public:
  IoThreadClientThrottle(int child_id,
                         int route_id,
                         net::URLRequest* request);
  virtual ~IoThreadClientThrottle();

  // From content::ResourceThrottle
  virtual void WillStartRequest(bool* defer) OVERRIDE;
  virtual void WillRedirectRequest(const GURL& new_url, bool* defer) OVERRIDE;

  bool MaybeDeferRequest(bool* defer);
  void OnIoThreadClientReady(int new_child_id, int new_route_id);
  bool MaybeBlockRequest();
  bool ShouldBlockRequest();
  int get_child_id() const { return child_id_; }
  int get_route_id() const { return route_id_; }

private:
  int child_id_;
  int route_id_;
  net::URLRequest* request_;
};

IoThreadClientThrottle::IoThreadClientThrottle(int child_id,
                                               int route_id,
                                               net::URLRequest* request)
    : child_id_(child_id),
      route_id_(route_id),
      request_(request) { }

IoThreadClientThrottle::~IoThreadClientThrottle() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  g_webview_resource_dispatcher_host_delegate.Get().
      RemovePendingThrottleOnIoThread(this);
}

void IoThreadClientThrottle::WillStartRequest(bool* defer) {
  if (route_id_ < 1) {
    // OPTIONS is used for preflighted requests which are generated internally.
    DCHECK_EQ("OPTIONS", request_->method());
    return;
  }
  DCHECK(child_id_);
  if (!MaybeDeferRequest(defer)) {
    MaybeBlockRequest();
  }
}

void IoThreadClientThrottle::WillRedirectRequest(const GURL& new_url,
                                                 bool* defer) {
  WillStartRequest(defer);
}

bool IoThreadClientThrottle::MaybeDeferRequest(bool* defer) {
  *defer = false;

  // Defer all requests of a pop up that is still not associated with Java
  // client so that the client will get a chance to override requests.
  scoped_ptr<XWalkContentsIoThreadClient> io_client =
      XWalkContentsIoThreadClient::FromID(child_id_, route_id_);
  if (io_client && io_client->PendingAssociation()) {
    *defer = true;
    XWalkResourceDispatcherHostDelegate::AddPendingThrottle(
        child_id_, route_id_, this);
  }
  return *defer;
}

void IoThreadClientThrottle::OnIoThreadClientReady(int new_child_id,
                                                   int new_route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

  if (!MaybeBlockRequest()) {
    controller()->Resume();
  }
}

bool IoThreadClientThrottle::MaybeBlockRequest() {
  if (ShouldBlockRequest()) {
    controller()->CancelWithError(net::ERR_ACCESS_DENIED);
    return true;
  }
  return false;
}

bool IoThreadClientThrottle::ShouldBlockRequest() {
  scoped_ptr<XWalkContentsIoThreadClient> io_client =
      XWalkContentsIoThreadClient::FromID(child_id_, route_id_);
  DCHECK(io_client.get());

  // Part of implementation of WebSettings.allowContentAccess.
  if (request_->url().SchemeIs(xwalk::kContentScheme) &&
      io_client->ShouldBlockContentUrls()) {
    return true;
  }

  // Part of implementation of WebSettings.allowFileAccess.
  if (request_->url().SchemeIsFile() &&
      io_client->ShouldBlockFileUrls()) {
    const GURL& url = request_->url();
    if (!url.has_path() ||
        // Application's assets and resources are always available.
        (url.path().find(xwalk::kAndroidResourcePath) != 0 &&
         url.path().find(xwalk::kAndroidAssetPath) != 0)) {
      return true;
    }
  }

  if (io_client->ShouldBlockNetworkLoads()) {
    if (request_->url().SchemeIs(chrome::kFtpScheme)) {
      return true;
    }
    SetCacheControlFlag(request_, net::LOAD_ONLY_FROM_CACHE);
  } else {
    XWalkContentsIoThreadClient::CacheMode cache_mode = io_client->GetCacheMode();
    switch(cache_mode) {
      case XWalkContentsIoThreadClient::LOAD_CACHE_ELSE_NETWORK:
        SetCacheControlFlag(request_, net::LOAD_PREFERRING_CACHE);
        break;
      case XWalkContentsIoThreadClient::LOAD_NO_CACHE:
        SetCacheControlFlag(request_, net::LOAD_BYPASS_CACHE);
        break;
      case XWalkContentsIoThreadClient::LOAD_CACHE_ONLY:
        SetCacheControlFlag(request_, net::LOAD_ONLY_FROM_CACHE);
        break;
      default:
        break;
    }
  }
  return false;
}

// static
void XWalkResourceDispatcherHostDelegate::ResourceDispatcherHostCreated() {
  content::ResourceDispatcherHost::Get()->SetDelegate(
      &g_webview_resource_dispatcher_host_delegate.Get());
}

XWalkResourceDispatcherHostDelegate::XWalkResourceDispatcherHostDelegate()
    : content::ResourceDispatcherHostDelegate() {
}

XWalkResourceDispatcherHostDelegate::~XWalkResourceDispatcherHostDelegate() {
}

void XWalkResourceDispatcherHostDelegate::RequestBeginning(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    appcache::AppCacheService* appcache_service,
    ResourceType::Type resource_type,
    int child_id,
    int route_id,
    bool is_continuation_of_transferred_request,
    ScopedVector<content::ResourceThrottle>* throttles) {
  // If io_client is NULL, then the browser side objects have already been
  // destroyed, so do not do anything to the request. Conversely if the
  // request relates to a not-yet-created popup window, then the client will
  // be non-NULL but PopupPendingAssociation() will be set.
  scoped_ptr<XWalkContentsIoThreadClient> io_client =
      XWalkContentsIoThreadClient::FromID(child_id, route_id);
  if (!io_client)
    return;

  throttles->push_back(new IoThreadClientThrottle(
      child_id, route_id, request));

  bool allow_intercepting =
      // We allow intercepting navigations within subframes, but only if the
      // scheme other than http or https. This is because the embedder
      // can't distinguish main frame and subframe callbacks (which could lead
      // to broken content if the embedder decides to not ignore the main frame
      // navigation, but ignores the subframe navigation).
      // The reason this is supported at all is that certain JavaScript-based
      // frameworks use iframe navigation as a form of communication with the
      // embedder.
      (resource_type == ResourceType::MAIN_FRAME ||
       (resource_type == ResourceType::SUB_FRAME &&
        !request->url().SchemeIs(content::kHttpScheme) &&
        !request->url().SchemeIs(content::kHttpsScheme)));
  if (allow_intercepting) {
    throttles->push_back(InterceptNavigationDelegate::CreateThrottleFor(
        request));
  }
}

void XWalkResourceDispatcherHostDelegate::DownloadStarting(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    int child_id,
    int route_id,
    int request_id,
    bool is_content_initiated,
    bool must_download,
    ScopedVector<content::ResourceThrottle>* throttles) {
  GURL url(request->url());
  std::string user_agent;
  std::string content_disposition;
  std::string mime_type;
  int64 content_length = request->GetExpectedContentSize();

  request->extra_request_headers().GetHeader(
      net::HttpRequestHeaders::kUserAgent, &user_agent);


  net::HttpResponseHeaders* response_headers = request->response_headers();
  if (response_headers) {
    response_headers->GetNormalizedHeader("content-disposition",
        &content_disposition);
    response_headers->GetMimeType(&mime_type);
  }

  request->Cancel();

  scoped_ptr<XWalkContentsIoThreadClient> io_client =
      XWalkContentsIoThreadClient::FromID(child_id, route_id);

  // POST request cannot be repeated in general, so prevent client from
  // retrying the same request, even if it is with a GET.
  if ("GET" == request->method() && io_client) {
    io_client->NewDownload(url,
                           user_agent,
                           content_disposition,
                           mime_type,
                           content_length);
  }
}

bool XWalkResourceDispatcherHostDelegate::AcceptAuthRequest(
    net::URLRequest* request,
    net::AuthChallengeInfo* auth_info) {
  return true;
}

content::ResourceDispatcherHostLoginDelegate*
    XWalkResourceDispatcherHostDelegate::CreateLoginDelegate(
        net::AuthChallengeInfo* auth_info,
        net::URLRequest* request) {
  return new XWalkLoginDelegate(auth_info, request);
}

bool XWalkResourceDispatcherHostDelegate::HandleExternalProtocol(const GURL& url,
                                                                 int child_id,
                                                                 int route_id) {
  NOTREACHED();
  return false;
}

void XWalkResourceDispatcherHostDelegate::OnResponseStarted(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    content::ResourceResponse* response,
    IPC::Sender* sender) {
  const content::ResourceRequestInfo* request_info =
      content::ResourceRequestInfo::ForRequest(request);
  if (!request_info) {
    DLOG(FATAL) << "Started request without associated info: " <<
        request->url();
    return;
  }

  if (request_info->GetResourceType() == ResourceType::MAIN_FRAME) {
    // Check for x-auto-login header.
    auto_login_parser::HeaderData header_data;
    if (auto_login_parser::ParserHeaderInResponse(
            request, auto_login_parser::ALLOW_ANY_REALM, &header_data)) {
      scoped_ptr<XWalkContentsIoThreadClient> io_client =
          XWalkContentsIoThreadClient::FromID(request_info->GetChildID(),
                                              request_info->GetRouteID());
      io_client->NewLoginRequest(
          header_data.realm, header_data.account, header_data.args);
    }
  }
}

void XWalkResourceDispatcherHostDelegate::RemovePendingThrottleOnIoThread(
    IoThreadClientThrottle* throttle) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  PendingThrottleMap::iterator it = pending_throttles_.find(
      ChildRouteIDPair(throttle->get_child_id(), throttle->get_route_id()));
  if (it != pending_throttles_.end()) {
    pending_throttles_.erase(it);
  }
}

// static
void XWalkResourceDispatcherHostDelegate::OnIoThreadClientReady(
    int new_child_id,
    int new_route_id) {
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(
          &XWalkResourceDispatcherHostDelegate::OnIoThreadClientReadyInternal,
          base::Unretained(
              g_webview_resource_dispatcher_host_delegate.Pointer()),
          new_child_id, new_route_id));
}

// static
void XWalkResourceDispatcherHostDelegate::AddPendingThrottle(
    int child_id,
    int route_id,
    IoThreadClientThrottle* pending_throttle) {
  BrowserThread::PostTask(BrowserThread::IO, FROM_HERE,
      base::Bind(
          &XWalkResourceDispatcherHostDelegate::AddPendingThrottleOnIoThread,
          base::Unretained(
              g_webview_resource_dispatcher_host_delegate.Pointer()),
          child_id, route_id, pending_throttle));
}

void XWalkResourceDispatcherHostDelegate::AddPendingThrottleOnIoThread(
    int child_id,
    int route_id,
    IoThreadClientThrottle* pending_throttle) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  pending_throttles_.insert(
      std::pair<ChildRouteIDPair, IoThreadClientThrottle*>(
          ChildRouteIDPair(child_id, route_id), pending_throttle));
}

void XWalkResourceDispatcherHostDelegate::OnIoThreadClientReadyInternal(
    int new_child_id,
    int new_route_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  PendingThrottleMap::iterator it = pending_throttles_.find(
      ChildRouteIDPair(new_child_id, new_route_id));

  if (it != pending_throttles_.end()) {
    IoThreadClientThrottle* throttle = it->second;
    throttle->OnIoThreadClientReady(new_child_id, new_route_id);
    pending_throttles_.erase(it);
  }
}

}  // namespace xwalk
