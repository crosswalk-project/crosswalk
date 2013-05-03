// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_quota_permission_context.h"

#include "webkit/quota/quota_types.h"

namespace content {

ShellQuotaPermissionContext::ShellQuotaPermissionContext() {}

void ShellQuotaPermissionContext::RequestQuotaPermission(
    const GURL& origin_url,
    quota::StorageType type,
    int64 requested_quota,
    int render_process_id,
    int render_view_id,
    const PermissionCallback& callback) {
  if (type != quota::kStorageTypePersistent) {
    // For now we only support requesting quota with this interface
    // for Persistent storage type.
    callback.Run(QUOTA_PERMISSION_RESPONSE_DISALLOW);
    return;
  }

  callback.Run(QUOTA_PERMISSION_RESPONSE_ALLOW);
}

ShellQuotaPermissionContext::~ShellQuotaPermissionContext() {}

}  // namespace content
