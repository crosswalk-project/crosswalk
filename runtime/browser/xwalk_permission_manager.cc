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

#if defined(OS_ANDROID) || defined(OS_TIZEN)
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

void XWalkPermissionManager::RequestPermission(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    int request_id,
    const GURL& requesting_origin,
    bool user_gesture,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  switch (permission) {
    case content::PermissionType::GEOLOCATION:
#if defined(OS_ANDROID) || defined(OS_TIZEN)
      if (!geolocation_permission_context_.get()) {
        geolocation_permission_context_ =
            new RuntimeGeolocationPermissionContext();
      }
      geolocation_permission_context_->RequestGeolocationPermission(
          render_frame_host,
          requesting_origin,
          base::Bind(&CallbackPermisisonStatusWrapper, callback));
#else
      callback.Run(content::PERMISSION_STATUS_DENIED);
#endif
      break;
    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
      callback.Run(content::PERMISSION_STATUS_GRANTED);
      break;
    case content::PermissionType::MIDI_SYSEX:
    case content::PermissionType::NOTIFICATIONS:
    case content::PermissionType::PUSH_MESSAGING:
      NOTIMPLEMENTED() << "RequestPermission is not implemented for "
                       << static_cast<int>(permission);
      callback.Run(content::PERMISSION_STATUS_DENIED);
      break;
    case content::PermissionType::NUM:
      NOTREACHED() << "PermissionType::NUM was not expected here.";
      callback.Run(content::PERMISSION_STATUS_DENIED);
      break;
  }
}

void XWalkPermissionManager::CancelPermissionRequest(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    int request_id,
    const GURL& requesting_origin) {
  switch (permission) {
    case content::PermissionType::GEOLOCATION:
#if defined(OS_ANDROID) || defined(OS_TIZEN)
      geolocation_permission_context_->CancelGeolocationPermissionRequest(
          render_frame_host, requesting_origin);
#endif
      break;
    case content::PermissionType::PROTECTED_MEDIA_IDENTIFIER:
      break;
    case content::PermissionType::MIDI_SYSEX:
    case content::PermissionType::NOTIFICATIONS:
    case content::PermissionType::PUSH_MESSAGING:
      NOTIMPLEMENTED() << "CancelPermission not implemented for "
                       << static_cast<int>(permission);
      break;
    case content::PermissionType::NUM:
      NOTREACHED() << "PermissionType::NUM was not expected here.";
      break;
  }
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
