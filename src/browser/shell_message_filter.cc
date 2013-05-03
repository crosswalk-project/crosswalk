// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_message_filter.h"

#include "base/file_util.h"
#include "base/threading/thread_restrictions.h"
#include "cameo/src/common/shell_messages.h"
#include "cameo/src/browser/shell_network_delegate.h"
#include "content/public/browser/child_process_security_policy.h"
#include "net/base/net_errors.h"
#include "webkit/database/database_tracker.h"
#include "webkit/fileapi/isolated_context.h"
#include "webkit/quota/quota_manager.h"

namespace content {

ShellMessageFilter::ShellMessageFilter(
    int render_process_id,
    webkit_database::DatabaseTracker* database_tracker,
    quota::QuotaManager* quota_manager)
    : render_process_id_(render_process_id),
      database_tracker_(database_tracker),
      quota_manager_(quota_manager) {
}

ShellMessageFilter::~ShellMessageFilter() {
}

bool ShellMessageFilter::OnMessageReceived(const IPC::Message& message,
                                           bool* message_was_ok) {
  bool handled = false;

  return handled;
}

}  // namespace content
