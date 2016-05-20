// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_permission_manager.h"

#include <string>
#include <vector>

#include "base/callback.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"

using blink::mojom::PermissionStatus;
using content::PermissionType;

namespace xwalk {

struct XWalkPermissionManager::PendingRequest {
 public:
  PendingRequest(PermissionType permission,
                 GURL requesting_origin,
                 GURL embedding_origin,
                 content::RenderFrameHost* render_frame_host,
                 const base::Callback<void(PermissionStatus)>& callback)
    : permission(permission),
      requesting_origin(requesting_origin),
      embedding_origin(embedding_origin),
      render_process_id(render_frame_host->GetProcess()->GetID()),
      render_frame_id(render_frame_host->GetRoutingID()),
      callback(callback) {
  }

  ~PendingRequest() = default;

  PermissionType permission;
  GURL requesting_origin;
  GURL embedding_origin;
  int render_process_id;
  int render_frame_id;
  base::Callback<void(PermissionStatus)> callback;
};

XWalkPermissionManager::XWalkPermissionManager(
    application::ApplicationService* application_service)
    : content::PermissionManager(),
      application_service_(application_service),
      weak_ptr_factory_(this) {
}

XWalkPermissionManager::~XWalkPermissionManager() {
}

void XWalkPermissionManager::GetApplicationName(
    content::RenderFrameHost* render_frame_host,
    std::string* name) {
  application::Application* app =
      application_service_->GetApplicationByRenderHostID(
      render_frame_host->GetProcess()->GetID());
  if (app)
    *name = app->data()->Name();
}

int XWalkPermissionManager::RequestPermission(
      content::PermissionType permission,
      content::RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      const base::Callback<void(PermissionStatus)>& callback) {
  bool should_delegate_request = true;
  for (PendingRequestsMap::Iterator<PendingRequest> it(&pending_requests_);
      !it.IsAtEnd(); it.Advance()) {
    if (permission == it.GetCurrentValue()->permission) {
      should_delegate_request = false;
      break;
    }
  }

  const GURL& embedding_origin =
      content::WebContents::FromRenderFrameHost(render_frame_host)
          ->GetLastCommittedURL().GetOrigin();
  int request_id = kNoPendingOperation;

  switch (permission) {
    case content::PermissionType::GEOLOCATION: {
      request_id = pending_requests_.Add(new PendingRequest(
          permission, requesting_origin,
          embedding_origin, render_frame_host,
          callback));

      if (should_delegate_request) {
        if (!geolocation_permission_context_.get()) {
          geolocation_permission_context_ =
              new RuntimeGeolocationPermissionContext();
        }
        std::string app_name;
        GetApplicationName(render_frame_host, &app_name);
        geolocation_permission_context_->RequestGeolocationPermission(
            content::WebContents::FromRenderFrameHost(render_frame_host),
            requesting_origin,
            app_name,
            base::Bind(&OnRequestResponse, weak_ptr_factory_.GetWeakPtr(),
                       request_id, callback));
      }
      break;
    }
    case content::PermissionType::NOTIFICATIONS: {
      request_id = pending_requests_.Add(new PendingRequest(
        permission, requesting_origin,
        embedding_origin, render_frame_host,
        callback));
      if (should_delegate_request) {
        if (!notification_permission_context_.get()) {
          notification_permission_context_ =
              new RuntimeNotificationPermissionContext();
        }
        std::string app_name;
        GetApplicationName(render_frame_host, &app_name);
        notification_permission_context_->RequestNotificationPermission(
            content::WebContents::FromRenderFrameHost(render_frame_host),
            requesting_origin,
            app_name,
            base::Bind(&OnRequestResponse, weak_ptr_factory_.GetWeakPtr(),
                request_id, callback));
      }
      break;
    }
    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
      callback.Run(PermissionStatus::GRANTED);
      break;
    case content::PermissionType::AUDIO_CAPTURE:
    case content::PermissionType::BACKGROUND_SYNC:
    case content::PermissionType::DURABLE_STORAGE:
    case content::PermissionType::MIDI:
    case content::PermissionType::MIDI_SYSEX:
    case content::PermissionType::PUSH_MESSAGING:
    case content::PermissionType::VIDEO_CAPTURE:
      NOTIMPLEMENTED() << "RequestPermission is not implemented for "
                       << static_cast<int>(permission);
      callback.Run(PermissionStatus::DENIED);
      break;
    case content::PermissionType::NUM:
      NOTREACHED() << "PermissionType::NUM was not expected here.";
      callback.Run(PermissionStatus::DENIED);
      break;
  }
  return request_id;
}

int XWalkPermissionManager::RequestPermissions(
    const std::vector<content::PermissionType>& permissions,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    const base::Callback<void(
        const std::vector<PermissionStatus>&)>& callback) {
  // TODO(mrunalk): Rework this as per,
  // https://codereview.chromium.org/1419083002
  NOTIMPLEMENTED() << "RequestPermissions not implemented in Crosswalk";
  return kNoPendingOperation;
}

void XWalkPermissionManager::CancelPermissionRequest(int request_id) {
  PendingRequest* pending_request = pending_requests_.Lookup(request_id);
  if (!pending_request)
    return;
  content::RenderFrameHost* render_frame_host =
      content::RenderFrameHost::FromID(pending_request->render_process_id,
          pending_request->render_frame_id);

  switch (pending_request->permission) {
    case content::PermissionType::GEOLOCATION:
      geolocation_permission_context_->CancelGeolocationPermissionRequest(
          content::WebContents::FromRenderFrameHost(render_frame_host),
          pending_request->requesting_origin);
      break;
    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
      break;
    case content::PermissionType::AUDIO_CAPTURE:
    case content::PermissionType::BACKGROUND_SYNC:
    case content::PermissionType::DURABLE_STORAGE:
    case content::PermissionType::MIDI:
    case content::PermissionType::MIDI_SYSEX:
    case content::PermissionType::NOTIFICATIONS:
    case content::PermissionType::PUSH_MESSAGING:
    case content::PermissionType::VIDEO_CAPTURE:
      NOTIMPLEMENTED() << "CancelPermission not implemented for "
                       << static_cast<int>(pending_request->permission);
      break;
    case content::PermissionType::NUM:
      NOTREACHED() << "PermissionType::NUM was not expected here.";
      break;
  }
  pending_requests_.Remove(request_id);
}

// static
void XWalkPermissionManager::OnRequestResponse(
    const base::WeakPtr<XWalkPermissionManager>& manager,
    int request_id,
    const base::Callback<void(PermissionStatus)>& callback,
    bool allowed) {
  PermissionStatus status = allowed ? PermissionStatus::GRANTED
                                    : PermissionStatus::DENIED;
  if (manager.get()) {
    PendingRequest* pending_request =
    manager->pending_requests_.Lookup(request_id);

    for (PendingRequestsMap::Iterator<PendingRequest> it(
            &manager->pending_requests_);
         !it.IsAtEnd(); it.Advance()) {
      if (pending_request->permission == it.GetCurrentValue()->permission &&
          it.GetCurrentKey() != request_id) {
        it.GetCurrentValue()->callback.Run(status);
        manager->pending_requests_.Remove(it.GetCurrentKey());
      }
    }

    manager->pending_requests_.Remove(request_id);
  }
  callback.Run(status);
}

void XWalkPermissionManager::ResetPermission(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
}

PermissionStatus XWalkPermissionManager::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  if (permission == content::PermissionType::PROTECTED_MEDIA_IDENTIFIER)
    return PermissionStatus::GRANTED;

  return PermissionStatus::DENIED;
}

void XWalkPermissionManager::RegisterPermissionUsage(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
}

int XWalkPermissionManager::SubscribePermissionStatusChange(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin,
    const base::Callback<void(PermissionStatus)>& callback) {
  return kNoPendingOperation;
}

void XWalkPermissionManager::UnsubscribePermissionStatusChange(
    int subscription_id) {
}

}  // namespace xwalk
