// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_RESOURCE_DISPATCHER_HOST_DELEGATE_ANDROID_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_RESOURCE_DISPATCHER_HOST_DELEGATE_ANDROID_H_

#include <map>
#include <utility>

#include "base/lazy_instance.h"
#include "content/public/browser/content_browser_client.h"
#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate.h"

namespace content {
class ResourceDispatcherHostLoginDelegate;
struct ResourceResponse;
}  // namespace content

namespace IPC {
class Sender;
}  // namespace IPC

namespace xwalk {

class IoThreadClientThrottle;

class RuntimeResourceDispatcherHostDelegateAndroid
    : public RuntimeResourceDispatcherHostDelegate {
 public:
  RuntimeResourceDispatcherHostDelegateAndroid();
  virtual ~RuntimeResourceDispatcherHostDelegateAndroid();

  static void ResourceDispatcherHostCreated();

  virtual void RequestBeginning(
      net::URLRequest* request,
      content::ResourceContext* resource_context,
      appcache::AppCacheService* appcache_service,
      ResourceType::Type resource_type,
      int child_id,
      int route_id,
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
  virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info,
      net::URLRequest* request) OVERRIDE;
  virtual bool HandleExternalProtocol(
      const GURL& url,
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
      RuntimeResourceDispatcherHostDelegateAndroid>;
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

  DISALLOW_COPY_AND_ASSIGN(RuntimeResourceDispatcherHostDelegateAndroid);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_RESOURCE_DISPATCHER_HOST_DELEGATE_ANDROID_H_
