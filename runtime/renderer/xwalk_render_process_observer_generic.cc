// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/xwalk_render_process_observer_generic.h"

#include <vector>

#include "content/renderer/render_thread_impl.h"
#include "content/renderer/renderer_webkitplatformsupport_impl.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"
#include "xwalk/runtime/common/xwalk_content_client.h"


namespace xwalk {
namespace {
struct AccessWhitelistItem {
  AccessWhitelistItem(
      const GURL& source, const GURL& dest, bool allow_subdomains);
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

std::vector<AccessWhitelistItem> access_whitelist;

void AddAccessWhiteListEntry(
    const GURL& source, const GURL& dest, bool allow_subdomains) {
  blink::WebSecurityPolicy::addOriginAccessWhitelistEntry(
      source.GetOrigin(),
      blink::WebString::fromUTF8(dest.scheme()),
      blink::WebString::fromUTF8(dest.HostNoBrackets()),
      allow_subdomains);
}
}  // namespace

XWalkRenderProcessObserver::XWalkRenderProcessObserver()
    : is_webkit_initialized_(false),
      is_suspended_(false),
      security_mode_(application::SecurityPolicy::NoSecurity) {
}

XWalkRenderProcessObserver::~XWalkRenderProcessObserver() {
}

bool XWalkRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderProcessObserver, message)
    IPC_MESSAGE_HANDLER(ViewMsg_SetAccessWhiteList, OnSetAccessWhiteList)
    IPC_MESSAGE_HANDLER(ViewMsg_EnableSecurityMode, OnEnableSecurityMode)
    IPC_MESSAGE_HANDLER(ViewMsg_SuspendJSEngine, OnSuspendJSEngine)
#if defined(OS_TIZEN)
    IPC_MESSAGE_HANDLER(ViewMsg_UserAgentStringChanged, OnUserAgentChanged)
#endif
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderProcessObserver::WebKitInitialized() {
  is_webkit_initialized_ = true;
  for (std::vector<AccessWhitelistItem>::iterator it = access_whitelist.begin();
       it != access_whitelist.end(); ++it)
    AddAccessWhiteListEntry(it->source_, it->dest_, it->allow_subdomains_);

  access_whitelist.clear();
}

void XWalkRenderProcessObserver::OnRenderProcessShutdown() {
  is_webkit_initialized_ = false;
}

void XWalkRenderProcessObserver::OnSetAccessWhiteList(const GURL& source,
                                                      const GURL& dest,
                                                      bool allow_subdomains) {
  if (is_webkit_initialized_)
    AddAccessWhiteListEntry(source, dest, allow_subdomains);
  else
    access_whitelist.push_back(
        AccessWhitelistItem(source, dest, allow_subdomains));
}

void XWalkRenderProcessObserver::OnEnableSecurityMode(
    const GURL& url, application::SecurityPolicy::SecurityMode mode) {
  app_url_ = url;
  security_mode_ = mode;
}

void XWalkRenderProcessObserver::OnSuspendJSEngine(bool is_suspend) {
  if (is_suspend == is_suspended_)
    return;
  content::RenderThreadImpl* thread = content::RenderThreadImpl::current();
  thread->EnsureWebKitInitialized();
  if (is_suspend)
    thread->webkit_platform_support()->SuspendSharedTimer();
  else
    thread->webkit_platform_support()->ResumeSharedTimer();
  is_suspended_ = is_suspend;
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

}  // namespace xwalk
