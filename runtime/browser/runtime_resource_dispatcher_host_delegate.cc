// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate.h"

#include "base/lazy_instance.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
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

namespace {
base::LazyInstance<xwalk::RuntimeResourceDispatcherHostDelegate>
    g_runtime_resource_dispatcher_host_delegate = LAZY_INSTANCE_INITIALIZER;
}

namespace xwalk {

RuntimeResourceDispatcherHostDelegate::RuntimeResourceDispatcherHostDelegate() {
}

RuntimeResourceDispatcherHostDelegate::
~RuntimeResourceDispatcherHostDelegate() {
}

// static
void RuntimeResourceDispatcherHostDelegate::ResourceDispatcherHostCreated() {
  content::ResourceDispatcherHost::Get()->SetDelegate(
      &g_runtime_resource_dispatcher_host_delegate.Get());
}

void RuntimeResourceDispatcherHostDelegate::RequestBeginning(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    appcache::AppCacheService* appcache_service,
    ResourceType::Type resource_type,
    int child_id,
    int route_id,
    ScopedVector<content::ResourceThrottle>* throttles) {
}

void RuntimeResourceDispatcherHostDelegate::DownloadStarting(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    int child_id,
    int route_id,
    int request_id,
    bool is_content_initiated,
    bool must_download,
    ScopedVector<content::ResourceThrottle>* throttles) {
}

bool RuntimeResourceDispatcherHostDelegate::HandleExternalProtocol(
    const GURL& url,
    int child_id,
    int route_id) {
  return true;
}

}  // namespace xwalk
