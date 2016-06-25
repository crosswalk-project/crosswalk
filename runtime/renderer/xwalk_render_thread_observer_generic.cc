// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/xwalk_render_thread_observer_generic.h"

#include "content/public/renderer/render_thread.h"
#include "extensions/common/url_pattern.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/platform/WebSecurityOrigin.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"
#include "xwalk/runtime/common/xwalk_content_client.h"


namespace xwalk {
struct AccessWhitelistItem {
  AccessWhitelistItem(const GURL& source,
                      const GURL& dest,
                      const std::string& dest_host,
                      bool allow_subdomains);
  GURL source;
  GURL dest;
  // Have host as a separate field in order to consider CSP directives ('*');
  std::string dest_host;
  bool allow_subdomains;
};

AccessWhitelistItem::AccessWhitelistItem(
    const GURL& source, const GURL& dest,
    const std::string& dest_host, bool allow_subdomains)
    : source(source),
      dest(dest),
      dest_host(dest_host),
      allow_subdomains(allow_subdomains) {
}


void XWalkRenderThreadObserver::AddAccessWhiteListEntry(
    const GURL& source,
    const GURL& dest,
    const std::string& dest_host,
    bool allow_subdomains) {
  blink::WebSecurityPolicy::addOriginAccessWhitelistEntry(
      source.GetOrigin(),
      blink::WebString::fromUTF8(dest.scheme()),
      blink::WebString::fromUTF8(dest_host),
      allow_subdomains);
}

XWalkRenderThreadObserver::XWalkRenderThreadObserver()
    : is_blink_initialized_(false),
      security_mode_(application::ApplicationSecurityPolicy::NoSecurity) {
}

XWalkRenderThreadObserver::~XWalkRenderThreadObserver() {
}

bool XWalkRenderThreadObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderThreadObserver, message)
    IPC_MESSAGE_HANDLER(ViewMsg_SetAccessWhiteList, OnSetAccessWhiteList)
    IPC_MESSAGE_HANDLER(ViewMsg_EnableSecurityMode, OnEnableSecurityMode)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderThreadObserver::OnRenderProcessShutdown() {
  is_blink_initialized_ = false;
}

void XWalkRenderThreadObserver::OnSetAccessWhiteList(
    const GURL& source, const GURL& dest,
    const std::string& dest_host, bool allow_subdomains) {
  base::AutoLock lock(lock_);
  if (is_blink_initialized_)
    AddAccessWhiteListEntry(source, dest, dest_host, allow_subdomains);

  access_whitelist_.push_back(
      new AccessWhitelistItem(source, dest, dest_host, allow_subdomains));
}

void XWalkRenderThreadObserver::OnEnableSecurityMode(
    const GURL& url,
    application::ApplicationSecurityPolicy::SecurityMode mode) {
  app_url_ = url;
  security_mode_ = mode;
}

bool XWalkRenderThreadObserver::CanRequest(const GURL& orig,
                                            const GURL& dest) const {
  if (!blink::WebSecurityOrigin::create(orig.GetOrigin()).canRequest(dest))
    return false;

  // Need to check the port.
  base::AutoLock lock(lock_);
  for (const auto& whitelist_entry : access_whitelist_) {
    if (whitelist_entry->source.GetOrigin() != orig.GetOrigin())
      continue;
    const GURL& dest_url = whitelist_entry->dest;
    if (!whitelist_entry->allow_subdomains &&
        whitelist_entry->dest.GetOrigin() == dest.GetOrigin() &&
        base::StartsWith(dest.path(), dest_url.path(),
                         base::CompareCase::INSENSITIVE_ASCII))
      return true;
    if (whitelist_entry->allow_subdomains &&
        dest.scheme() == dest_url.scheme() &&
        dest.DomainIs(dest_url.host().c_str()) &&
        dest.port() == dest_url.port() &&
        base::StartsWith(dest.path(), dest_url.path(),
                         base::CompareCase::INSENSITIVE_ASCII))
      return true;
  }
  return false;
}

}  // namespace xwalk
