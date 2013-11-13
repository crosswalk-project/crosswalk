// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_geolocation_permission_context.h"

#include "base/bind.h"
#include "base/callback.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_content.h"
#endif

namespace xwalk {

RuntimeGeolocationPermissionContext::~RuntimeGeolocationPermissionContext() {
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermissionOnUIThread(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    const GURL& requesting_frame,
    base::Callback<void(bool)> callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

#if defined(OS_ANDROID)
  XWalkContent* xwalk_content =
      XWalkContent::FromID(render_process_id, render_view_id);
  if (!xwalk_content) {
    callback.Run(false);
    return;
  }

  xwalk_content->ShowGeolocationPrompt(requesting_frame, callback);
#endif
  // TODO(yongsheng): Handle this for other platforms.
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermission(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    const GURL& requesting_frame,
    const base::Callback<void(bool)> callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &RuntimeGeolocationPermissionContext::
              RequestGeolocationPermissionOnUIThread,
          this,
          render_process_id,
          render_view_id,
          bridge_id,
          requesting_frame,
          callback));
}

content::GeolocationPermissionContext*
RuntimeGeolocationPermissionContext::Create(RuntimeContext* runtime_context) {
  return new RuntimeGeolocationPermissionContext();
}

void
RuntimeGeolocationPermissionContext
    ::CancelGeolocationPermissionRequestOnUIThread(
        int render_process_id,
        int render_view_id,
        int bridge_id,
        const GURL& requesting_frame) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

#if defined(OS_ANDROID)
  XWalkContent* xwalk_content =
      XWalkContent::FromID(render_process_id, render_view_id);
  if (xwalk_content) {
      xwalk_content->HideGeolocationPrompt(requesting_frame);
  }
#endif
  // TODO(yongsheng): Handle this for other platforms.
}

void
RuntimeGeolocationPermissionContext::CancelGeolocationPermissionRequest(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    const GURL& requesting_frame) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &RuntimeGeolocationPermissionContext::
              CancelGeolocationPermissionRequestOnUIThread,
          this,
          render_process_id,
          render_view_id,
          bridge_id,
          requesting_frame));
}

}  // namespace xwalk
