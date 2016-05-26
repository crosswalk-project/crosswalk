// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_PERMISSION_MANAGER_H_
#define XWALK_RUNTIME_BROWSER_XWALK_PERMISSION_MANAGER_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/id_map.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/permission_manager.h"
#include "xwalk/runtime/browser/runtime_geolocation_permission_context.h"
#include "xwalk/runtime/browser/runtime_notification_permission_context.h"

namespace xwalk {

namespace application {
class ApplicationService;
}

class XWalkPermissionManager : public content::PermissionManager {
 public:
  XWalkPermissionManager(
      application::ApplicationService* application_service);
  ~XWalkPermissionManager() override;

  // PermissionManager implementation.
  int RequestPermission(
      content::PermissionType permission,
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      const base::Callback<void(blink::mojom::PermissionStatus)>& callback)
      override;
  int RequestPermissions(
      const std::vector<content::PermissionType>& permissions,
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      const base::Callback<void(
          const std::vector<blink::mojom::PermissionStatus>&)>& callback)
          override;
  void CancelPermissionRequest(int request_id) override;
  void ResetPermission(content::PermissionType permission,
                       const GURL& requesting_origin,
                       const GURL& embedding_origin) override;
  blink::mojom::PermissionStatus GetPermissionStatus(
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
      const base::Callback<void(blink::mojom::PermissionStatus)>& callback)
      override;
  void UnsubscribePermissionStatusChange(int subscription_id) override;

 private:
  struct PendingRequest;
  using PendingRequestsMap = IDMap<PendingRequest, IDMapOwnPointer>;

  void GetApplicationName(
      content::RenderFrameHost* render_frame_host,
      std::string* name);
  static void OnRequestResponse(
      const base::WeakPtr<XWalkPermissionManager>& manager,
      int request_id,
      const base::Callback<void(blink::mojom::PermissionStatus)>& callback,
      bool allowed);

  PendingRequestsMap pending_requests_;
  scoped_refptr<RuntimeGeolocationPermissionContext>
      geolocation_permission_context_;
  scoped_refptr<RuntimeNotificationPermissionContext>
      notification_permission_context_;
  application::ApplicationService* application_service_;
  base::WeakPtrFactory<XWalkPermissionManager> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(XWalkPermissionManager);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_PERMISSION_MANAGER_H_
