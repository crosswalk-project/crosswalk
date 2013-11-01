// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/android/xwalk_render_process_observer.h"

#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebCache.h"
#include "third_party/WebKit/public/web/WebNetworkStateNotifier.h"
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
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkRenderProcessObserver::WebKitInitialized() {
  webkit_initialized_ = true;
}

void XWalkRenderProcessObserver::OnSetJsOnlineProperty(bool network_up) {
  if (webkit_initialized_)
    WebKit::WebNetworkStateNotifier::setOnLine(network_up);
}

void XWalkRenderProcessObserver::OnClearCache() {
  if (webkit_initialized_)
    WebKit::WebCache::clear();
}

}  // namespace xwalk
