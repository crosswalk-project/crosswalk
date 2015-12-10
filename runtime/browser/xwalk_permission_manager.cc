// Copyright 2015 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_permission_manager.h"

#include "base/callback.h"
#include "content/public/browser/permission_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"

namespace xwalk {

namespace {

#if defined(OS_ANDROID)
void CallbackPermisisonStatusWrapper(
    const base::Callback<void(content::PermissionStatus)>& callback,
    bool allowed) {
  callback.Run(allowed ? content::PERMISSION_STATUS_GRANTED
                       : content::PERMISSION_STATUS_DENIED);
}
#endif

}  // anonymous namespace

XWalkPermissionManager::XWalkPermissionManager()
    : content::PermissionManager() {
}

XWalkPermissionManager::~XWalkPermissionManager() {
}

int XWalkPermissionManager::RequestPermission(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool user_gesture,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  int request_id = kNoPendingOperation;
  switch (permission) {
    case content::PermissionType::GEOLOCATION:
#if defined(OS_ANDROID)
      if (!geolocation_permission_context_.get()) {
        geolocation_permission_context_ =
            new RuntimeGeolocationPermissionContext();
      }
      geolocation_permission_context_->RequestGeolocationPermission(
          content::WebContents::FromRenderFrameHost(render_frame_host),
          requesting_origin,
          base::Bind(&CallbackPermisisonStatusWrapper, callback));
#else
      callback.Run(content::PERMISSION_STATUS_DENIED);
#endif
      break;
    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
      callback.Run(content::PERMISSION_STATUS_GRANTED);
      break;
    case content::PermissionType::AUDIO_CAPTURE:
    case content::PermissionType::VIDEO_CAPTURE:
    case content::PermissionType::MIDI_SYSEX:
    case content::PermissionType::NOTIFICATIONS:
    case content::PermissionType::PUSH_MESSAGING:
    case content::PermissionType::MIDI:
    case content::PermissionType::DURABLE_STORAGE:
      NOTIMPLEMENTED() << "RequestPermission is not implemented for "
                       << static_cast<int>(permission);
      callback.Run(content::PERMISSION_STATUS_DENIED);
      break;
    case content::PermissionType::NUM:
      NOTREACHED() << "PermissionType::NUM was not expected here.";
      callback.Run(content::PERMISSION_STATUS_DENIED);
      break;
  }
  return request_id;
}

int XWalkPermissionManager::RequestPermissions(
    const std::vector<content::PermissionType>& permissions,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool user_gesture,
    const base::Callback<void(
        const std::vector<content::PermissionStatus>&)>& callback) {
// TODO(mrunalk): Rework this as per, https://codereview.chromium.org/1419083002
  NOTIMPLEMENTED() << "RequestPermissions not implemented in Crosswalk";
  return kNoPendingOperation;
}

void XWalkPermissionManager::CancelPermissionRequest(int request_id) {
// TODO(mrunalk): Rework this as per,
// https://codereview.chromium.org/1342833002
// https://codereview.chromium.org/1375563002
//  switch (permission) {
//    case content::PermissionType::GEOLOCATION:
//#if defined(OS_ANDROID)
//      geolocation_permission_context_->CancelGeolocationPermissionRequest(
//          content::WebContents::FromRenderFrameHost(render_frame_host),
//          requesting_origin);
//#endif
//      break;
//    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
//      break;
//    case content::PermissionType::MIDI_SYSEX:
//    case content::PermissionType::NOTIFICATIONS:
//    case content::PermissionType::PUSH_MESSAGING:
//    case content::PermissionType::MIDI:
//    case content::PermissionType::DURABLE_STORAGE:
//      NOTIMPLEMENTED() << "CancelPermission not implemented for "
//                       << static_cast<int>(permission);
//      break;
//    case content::PermissionType::NUM:
//      NOTREACHED() << "PermissionType::NUM was not expected here.";
//      break;
//  }
}

void XWalkPermissionManager::ResetPermission(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
}

content::PermissionStatus XWalkPermissionManager::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  if (permission == content::PermissionType::PROTECTED_MEDIA_IDENTIFIER)
    return content::PERMISSION_STATUS_GRANTED;

  return content::PERMISSION_STATUS_DENIED;
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
    const base::Callback<void(content::PermissionStatus)>& callback) {
  return -1;
}

void XWalkPermissionManager::UnsubscribePermissionStatusChange(
    int subscription_id) {
}

}  // namespace xwalk
