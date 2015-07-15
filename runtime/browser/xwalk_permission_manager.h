// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PERMISSION_MANAGER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PERMISSION_MANAGER_H_

#include "base/callback_forward.h"
#include "base/macros.h"
#include "content/public/browser/permission_manager.h"
#include "xwalk/runtime/browser/runtime_geolocation_permission_context.h"

namespace xwalk {

class XWalkPermissionManager : public content::PermissionManager {
 public:
  XWalkPermissionManager();
  ~XWalkPermissionManager() override;

  // PermissionManager implementation.
  void RequestPermission(
      content::PermissionType permission,
      content::RenderFrameHost* render_frame_host,
      int request_id,
      const GURL& requesting_origin,
      bool user_gesture,
      const base::Callback<void(content::PermissionStatus)>& callback) override;
  void CancelPermissionRequest(content::PermissionType permission,
                               content::RenderFrameHost* render_frame_host,
                               int request_id,
                               const GURL& requesting_origin) override;
  void ResetPermission(content::PermissionType permission,
                       const GURL& requesting_origin,
                       const GURL& embedding_origin) override;
  content::PermissionStatus GetPermissionStatus(
      content::PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin) override;
  void RegisterPermissionUsage(content::PermissionType permission,
                               const GURL& requesting_origin,
                               const GURL& embedding_origin) override;
  int SubscribePermissionStatusChange(
      content::PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin,
      const base::Callback<void(content::PermissionStatus)>& callback) override;
  void UnsubscribePermissionStatusChange(int subscription_id) override;

 private:
  scoped_refptr<RuntimeGeolocationPermissionContext>
    geolocation_permission_context_;

  DISALLOW_COPY_AND_ASSIGN(XWalkPermissionManager);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PERMISSION_MANAGER_H_
