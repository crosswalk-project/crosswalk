// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_cookie_access_policy.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_thread.h"

using base::AutoLock;
using content::BrowserThread;

namespace xwalk {

namespace {
base::LazyInstance<XWalkCookieAccessPolicy>::Leaky g_lazy_instance;
}  // namespace

XWalkCookieAccessPolicy::~XWalkCookieAccessPolicy() {
}

XWalkCookieAccessPolicy::XWalkCookieAccessPolicy()
    : allow_access_(true) {
}

XWalkCookieAccessPolicy* XWalkCookieAccessPolicy::GetInstance() {
  return g_lazy_instance.Pointer();
}

bool XWalkCookieAccessPolicy::GetGlobalAllowAccess() {
  AutoLock lock(lock_);
  return allow_access_;
}

void XWalkCookieAccessPolicy::SetGlobalAllowAccess(bool allow) {
  AutoLock lock(lock_);
  allow_access_ = allow;
}

bool XWalkCookieAccessPolicy::OnCanGetCookies(
    const net::URLRequest& request,
    const net::CookieList& cookie_list) {
  return GetGlobalAllowAccess();
}

bool XWalkCookieAccessPolicy::OnCanSetCookie(
    const net::URLRequest& request,
    const std::string& cookie_line,
    net::CookieOptions* options) {
  return GetGlobalAllowAccess();
}

bool XWalkCookieAccessPolicy::AllowGetCookie(
    const GURL& url,
    const GURL& first_party,
    const net::CookieList& cookie_list,
    content::ResourceContext* context,
    int render_process_id,
    int render_view_id) {
  return GetGlobalAllowAccess();
}

bool XWalkCookieAccessPolicy::AllowSetCookie(
    const GURL& url,
    const GURL& first_party,
    const std::string& cookie_line,
    content::ResourceContext* context,
    int render_process_id,
    int render_view_id,
    net::CookieOptions* options) {
  return GetGlobalAllowAccess();
}

}  // namespace xwalk
