// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace xwalk {

class RuntimeResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  RuntimeResourceDispatcherHostDelegate();
  virtual ~RuntimeResourceDispatcherHostDelegate();

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
  virtual bool HandleExternalProtocol(
      const GURL& url,
      int child_id,
      int route_id) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(RuntimeResourceDispatcherHostDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
