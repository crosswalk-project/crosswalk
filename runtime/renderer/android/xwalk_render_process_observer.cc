// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/android/xwalk_render_process_observer.h"

#include "base/json/json_reader.h"
#include "base/values.h"
#include "content/public/common/url_constants.h"
#include "extensions/common/url_pattern.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebCache.h"
#include "third_party/WebKit/public/web/WebNetworkStateNotifier.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "xwalk/runtime/browser/android/net/url_constants.h"
#include "xwalk/runtime/common/android/xwalk_render_view_messages.h"

namespace xwalk {

XWalkRenderProcessObserver::XWalkRenderProcessObserver()
  : webkit_initialized_(false) {
}

XWalkRenderProcessObserver::~XWalkRenderProcessObserver() {
}

bool XWalkRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkRenderProcessObserver, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_SetJsOnlineProperty, OnSetJsOnlineProperty)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_ClearCache, OnClearCache);
    IPC_MESSAGE_HANDLER(XWalkViewMsg_SetOriginAccessWhitelist,
                        OnSetOriginAccessWhitelist)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderProcessObserver::WebKitInitialized() {
  webkit_initialized_ = true;
}

void XWalkRenderProcessObserver::OnSetJsOnlineProperty(bool network_up) {
  if (webkit_initialized_)
    blink::WebNetworkStateNotifier::setOnLine(network_up);
}

void XWalkRenderProcessObserver::OnClearCache() {
  if (webkit_initialized_)
    blink::WebCache::clear();
}

void XWalkRenderProcessObserver::OnSetOriginAccessWhitelist(
    std::string base_url,
    std::string match_patterns) {
  blink::WebSecurityPolicy::resetOriginAccessWhitelists();

  DCHECK(!base_url.empty());
  if (base_url.empty() || match_patterns.empty())
    return;

  base::Value* patterns = base::JSONReader::Read(match_patterns);
  if (!patterns)
    return;

  base::ListValue* match_pattern_list = NULL;
  if (!patterns->GetAsList(&match_pattern_list))
    return;

  const char* schemes[] = {
    content::kHttpScheme,
    content::kHttpsScheme,
    content::kFileScheme,
    xwalk::kAppScheme,
  };
  size_t size = match_pattern_list->GetSize();
  for (size_t i = 0; i < size; i ++) {
    std::string match_pattern;
    if (!match_pattern_list->GetString(i, &match_pattern))
      continue;

    URLPattern allowedUrl(URLPattern::SCHEME_ALL);
    if (allowedUrl.Parse(match_pattern) != URLPattern::PARSE_SUCCESS)
      continue;

    for (size_t j = 0; j < arraysize(schemes); ++j) {
      if (allowedUrl.MatchesScheme(schemes[j])) {
        blink::WebSecurityPolicy::addOriginAccessWhitelistEntry(
              blink::WebURL(GURL(base_url)),
              blink::WebString::fromUTF8(schemes[j]),
              blink::WebString::fromUTF8(allowedUrl.host()),
              allowedUrl.match_subdomains());
      }
    }
  }
}

}  // namespace xwalk
