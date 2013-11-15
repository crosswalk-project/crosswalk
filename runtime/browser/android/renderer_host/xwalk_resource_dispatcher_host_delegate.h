// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_XWALK_RENDERER_VIEW_HOST_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define ANDROID_XWALK_RENDERER_VIEW_HOST_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include <map>

#include "base/lazy_instance.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace content {
class ResourceDispatcherHostLoginDelegate;
struct ResourceResponse;
}  // namespace content

namespace IPC {
class Sender;
}  // namespace IPC

namespace xwalk {

class IoThreadClientThrottle;

class XWalkResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  static void ResourceDispatcherHostCreated();

  // Overriden methods from ResourceDispatcherHostDelegate.
  virtual void RequestBeginning(
      net::URLRequest* request,
      content::ResourceContext* resource_context,
      appcache::AppCacheService* appcache_service,
      ResourceType::Type resource_type,
      int child_id,
      int route_id,
      bool is_continuation_of_transferred_request,
      ScopedVector<content::ResourceThrottle>* throttles) OVERRIDE;
  virtual void DownloadStarting(
      net::URLRequest* request,
      content::ResourceContext* resource_context,
      int child_id,
      int route_id,
      int request_id,
      bool is_content_initiated,
      bool must_download,
      ScopedVector<content::ResourceThrottle>* throttles) OVERRIDE;
  virtual bool AcceptAuthRequest(net::URLRequest* request,
                                 net::AuthChallengeInfo* auth_info) OVERRIDE;
  virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info,
      net::URLRequest* request) OVERRIDE;
  virtual bool HandleExternalProtocol(const GURL& url,
                                      int child_id,
                                      int route_id) OVERRIDE;
  virtual void OnResponseStarted(
      net::URLRequest* request,
      content::ResourceContext* resource_context,
      content::ResourceResponse* response,
      IPC::Sender* sender) OVERRIDE;

  void RemovePendingThrottleOnIoThread(IoThreadClientThrottle* throttle);

  static void OnIoThreadClientReady(int new_child_id, int new_route_id);
  static void AddPendingThrottle(int child_id,
                                 int route_id,
                                 IoThreadClientThrottle* pending_throttle);

 private:
  friend struct base::DefaultLazyInstanceTraits<
      XWalkResourceDispatcherHostDelegate>;
  XWalkResourceDispatcherHostDelegate();
  virtual ~XWalkResourceDispatcherHostDelegate();

  // These methods must be called on IO thread.
  void OnIoThreadClientReadyInternal(int child_id, int route_id);
  void AddPendingThrottleOnIoThread(int child_id,
                                    int route_id,
                                    IoThreadClientThrottle* pending_throttle);

  typedef std::pair<int, int> ChildRouteIDPair;
  typedef std::map<ChildRouteIDPair, IoThreadClientThrottle*>
      PendingThrottleMap;

  // Only accessed on the IO thread.
  PendingThrottleMap pending_throttles_;

  DISALLOW_COPY_AND_ASSIGN(XWalkResourceDispatcherHostDelegate);
};

}  // namespace xwalk

#endif  // ANDROID_XWALK_RENDERER_VIEW_HOST_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
