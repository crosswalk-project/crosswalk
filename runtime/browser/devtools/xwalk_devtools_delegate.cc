// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/devtools/xwalk_devtools_delegate.h"

#include <string>

#include "base/base64.h"
#include "base/memory/ref_counted_memory.h"
#include "base/strings/utf_string_conversions.h"
#include "base/thread_task_runner_handle.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/url_constants.h"
#include "net/test/embedded_test_server/tcp_listen_socket.h"

namespace xwalk {

XWalkDevToolsDelegate::XWalkDevToolsDelegate(XWalkBrowserContext* context)
    : browser_context_(context) {
}

XWalkDevToolsDelegate::~XWalkDevToolsDelegate() {
}

base::DictionaryValue* XWalkDevToolsDelegate::HandleCommand(
    content::DevToolsAgentHost* agent_host,
    base::DictionaryValue* command_dict) {
  return NULL;
}

void XWalkDevToolsDelegate::OnNewRuntimeAdded(Runtime* runtime) {
  runtime->set_observer(this);
  runtime->set_ui_delegate(DefaultRuntimeUIDelegate::Create(runtime));
  runtime->Show();
}

void XWalkDevToolsDelegate::OnRuntimeClosed(Runtime* runtime) {
  delete runtime;
}

}  // namespace xwalk
