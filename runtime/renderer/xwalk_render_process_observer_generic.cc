// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/xwalk_render_process_observer_generic.h"

#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebSecurityOrigin.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"
#include "xwalk/runtime/common/xwalk_content_client.h"


namespace xwalk {
struct AccessWhitelistItem {
  AccessWhitelistItem(const GURL& source,
                      const GURL& dest,
                      bool allow_subdomains);
  GURL source_;
  GURL dest_;
  bool allow_subdomains_;
};

AccessWhitelistItem::AccessWhitelistItem(
    const GURL& source, const GURL& dest, bool allow_subdomains)
    : source_(source),
      dest_(dest),
      allow_subdomains_(allow_subdomains) {
}


void XWalkRenderProcessObserver::AddAccessWhiteListEntry(
    const GURL& source, const GURL& dest, bool allow_subdomains) {
  blink::WebSecurityPolicy::addOriginAccessWhitelistEntry(
      source.GetOrigin(),
      blink::WebString::fromUTF8(dest.scheme()),
      blink::WebString::fromUTF8(dest.HostNoBrackets()),
      allow_subdomains);
}

XWalkRenderProcessObserver::XWalkRenderProcessObserver()
    : is_blink_initialized_(false),
      security_mode_(application::ApplicationSecurityPolicy::NoSecurity) {
}

XWalkRenderProcessObserver::~XWalkRenderProcessObserver() {
}

bool XWalkRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderProcessObserver, message)
    IPC_MESSAGE_HANDLER(ViewMsg_SetAccessWhiteList, OnSetAccessWhiteList)
    IPC_MESSAGE_HANDLER(ViewMsg_EnableSecurityMode, OnEnableSecurityMode)
#if defined(OS_TIZEN)
    IPC_MESSAGE_HANDLER(ViewMsg_UserAgentStringChanged, OnUserAgentChanged)
#endif
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderProcessObserver::WebKitInitialized() {
  base::AutoLock lock(lock_);
  is_blink_initialized_ = true;
  for (const auto& it : access_whitelist_)
    AddAccessWhiteListEntry(it->source_, it->dest_, it->allow_subdomains_);
}

void XWalkRenderProcessObserver::OnRenderProcessShutdown() {
  is_blink_initialized_ = false;
}

void XWalkRenderProcessObserver::OnSetAccessWhiteList(const GURL& source,
                                                      const GURL& dest,
                                                      bool allow_subdomains) {
  base::AutoLock lock(lock_);
  if (is_blink_initialized_)
    AddAccessWhiteListEntry(source, dest, allow_subdomains);

  access_whitelist_.push_back(
      new AccessWhitelistItem(source, dest, allow_subdomains));
}

void XWalkRenderProcessObserver::OnEnableSecurityMode(
    const GURL& url,
    application::ApplicationSecurityPolicy::SecurityMode mode) {
  app_url_ = url;
  security_mode_ = mode;
}

#if defined(OS_TIZEN)
void XWalkRenderProcessObserver::OnUserAgentChanged(
    const std::string& userAgentString) {
  overriden_user_agent_ = userAgentString;
}

std::string XWalkRenderProcessObserver::GetOverridenUserAgent() const {
  return overriden_user_agent_;
}
#endif

bool XWalkRenderProcessObserver::CanRequest(const GURL& orig,
                                            const GURL& dest) const {
  if (!blink::WebSecurityOrigin::create(orig.GetOrigin()).canRequest(dest))
    return false;

  // Need to check the port.
  base::AutoLock lock(lock_);
  for (const auto& whitelist_entry : access_whitelist_) {
    if (whitelist_entry->source_.GetOrigin() != orig.GetOrigin())
      continue;
    if (!whitelist_entry->allow_subdomains_ &&
        whitelist_entry->dest_.GetOrigin() == dest.GetOrigin() &&
        base::StartsWithASCII(
            dest.path(), whitelist_entry->dest_.path(), false))
      return true;
    const GURL& rule = whitelist_entry->dest_;
    if (whitelist_entry->allow_subdomains_ &&
        dest.scheme() == rule.scheme() &&
        dest.DomainIs(rule.host().c_str()) &&
        dest.port() == rule.port() &&
        base::StartsWithASCII(dest.path(), rule.path(), false))
      return true;
  }
  return false;
}

}  // namespace xwalk
