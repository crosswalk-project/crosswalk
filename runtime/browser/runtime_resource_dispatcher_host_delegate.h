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
  ~RuntimeResourceDispatcherHostDelegate() override;

  static void ResourceDispatcherHostCreated();
  static std::unique_ptr<RuntimeResourceDispatcherHostDelegate> Create();

  void RequestBeginning(
      net::URLRequest* request,
      content::ResourceContext* resource_context,
      content::AppCacheService* appcache_service,
      content::ResourceType resource_type,
      ScopedVector<content::ResourceThrottle>* throttles) override;
  void DownloadStarting(
      net::URLRequest* request,
      content::ResourceContext* resource_context,
      int child_id,
      int route_id,
      int request_id,
      bool is_content_initiated,
      bool must_download,
      ScopedVector<content::ResourceThrottle>* throttles) override;
  bool HandleExternalProtocol(
      const GURL& url,
      int child_id,
      const content::ResourceRequestInfo::WebContentsGetter&
          web_contents_getter,
      bool is_main_frame,
      ui::PageTransition page_transition,
      bool has_user_gesture) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(RuntimeResourceDispatcherHostDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
