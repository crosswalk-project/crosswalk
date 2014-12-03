// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_geolocation_permission_context.h"

#include "base/bind.h"
#include "base/callback.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/xwalk_content.h"
#endif

#if defined(OS_TIZEN)
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/permissions_handler.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/browser/xwalk_runner_tizen.h"
#endif

namespace xwalk {

void CancelGeolocationPermissionRequestOnUIThread(
        int render_process_id,
        int render_view_id,
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

void CancelGeolocationPermissionRequest(
    int render_process_id,
    int render_view_id,
    const GURL& requesting_frame) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &CancelGeolocationPermissionRequestOnUIThread,
          render_process_id,
          render_view_id,
          requesting_frame));
}

RuntimeGeolocationPermissionContext::~RuntimeGeolocationPermissionContext() {
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermissionOnUIThread(
    content::WebContents* web_contents,
    const GURL& requesting_frame,
    base::Callback<void(bool)> result_callback,
    base::Closure* cancel_callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  int render_process_id = web_contents->GetRenderProcessHost()->GetID();
  int render_view_id = web_contents->GetRenderViewHost()->GetRoutingID();

#if defined(OS_ANDROID)
  XWalkContent* xwalk_content =
      XWalkContent::FromID(render_process_id, render_view_id);
  if (!xwalk_content) {
    result_callback.Run(false);
    return;
  }

  xwalk_content->ShowGeolocationPrompt(requesting_frame, result_callback);
#elif defined(OS_TIZEN)
  bool has_geolocation_permission = false;
  XWalkRunner* runner = XWalkRunner::GetInstance();
  application::ApplicationSystem* app_system = runner->app_system();
  application::ApplicationService* app_service =
      app_system->application_service();
  application::Application* application =
      app_service->GetApplicationByRenderHostID(render_view_id);

  if (application) {
    application::StoredPermission per = application->GetPermission(
        application::SESSION_PERMISSION,
        application_manifest_permissions::kPermissionGeolocation);
    switch (per) {
      case application::StoredPermission::ALLOW:
        has_geolocation_permission = true;
        break;
      default:
        break;
    }
  }

  result_callback.Run(has_geolocation_permission);
#endif

  if (cancel_callback) {
     *cancel_callback = base::Bind(
         CancelGeolocationPermissionRequest,
         render_process_id,
         render_view_id,
         requesting_frame);
  }

  // TODO(yongsheng): Handle this for other platforms.
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermission(
    content::WebContents* web_contents,
    const GURL& requesting_frame,
    base::Callback<void(bool)> result_callback,
    base::Closure* cancel_callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &RuntimeGeolocationPermissionContext::
              RequestGeolocationPermissionOnUIThread,
          this,
          web_contents,
          requesting_frame,
          result_callback,
          cancel_callback));
}

}  // namespace xwalk
