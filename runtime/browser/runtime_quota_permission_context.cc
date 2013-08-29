// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_quota_permission_context.h"

#include "webkit/common/quota/quota_types.h"

namespace xwalk {

RuntimeQuotaPermissionContext::RuntimeQuotaPermissionContext() {}

void RuntimeQuotaPermissionContext::RequestQuotaPermission(
    const GURL& origin_url,
    quota::StorageType type,
    int64 requested_quota,
    int render_process_id,
    int render_view_id,
    const PermissionCallback& callback) {
  // TODO(wang16): Handle request according to app's manifest declaration.
  callback.Run(QUOTA_PERMISSION_RESPONSE_ALLOW);
}

RuntimeQuotaPermissionContext::~RuntimeQuotaPermissionContext() {}

}  // namespace xwalk
