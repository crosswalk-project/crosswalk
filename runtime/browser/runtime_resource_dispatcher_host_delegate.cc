// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate.h"

#include <memory>

#include "base/memory/ptr_util.h"
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
#include "xwalk/runtime/browser/runtime_platform_util.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate_android.h"
#endif

namespace xwalk {

// static
std::unique_ptr<RuntimeResourceDispatcherHostDelegate>
RuntimeResourceDispatcherHostDelegate::Create() {
#if defined(OS_ANDROID)
  return base::WrapUnique(
      static_cast<RuntimeResourceDispatcherHostDelegate*>(
            new RuntimeResourceDispatcherHostDelegateAndroid()));
#else
  return base::WrapUnique(new RuntimeResourceDispatcherHostDelegate());
#endif
}

RuntimeResourceDispatcherHostDelegate::RuntimeResourceDispatcherHostDelegate() {
}

RuntimeResourceDispatcherHostDelegate::
~RuntimeResourceDispatcherHostDelegate() {
}

void RuntimeResourceDispatcherHostDelegate::RequestBeginning(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    content::AppCacheService* appcache_service,
    content::ResourceType resource_type,
    ScopedVector<content::ResourceThrottle>* throttles) {
}

void RuntimeResourceDispatcherHostDelegate::DownloadStarting(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    int child_id,
    int route_id,
    bool is_content_initiated,
    bool must_download,
    ScopedVector<content::ResourceThrottle>* throttles) {
}

bool RuntimeResourceDispatcherHostDelegate::HandleExternalProtocol(
    const GURL& url,
    int child_id,
    const content::ResourceRequestInfo::WebContentsGetter& web_contents_getter,
    bool is_main_frame,
    ui::PageTransition page_transition,
    bool has_user_gesture,
    content::ResourceContext* resource_context) {
  platform_util::OpenExternal(url);
  return true;
}

}  // namespace xwalk
