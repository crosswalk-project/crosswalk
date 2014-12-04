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
#endif

namespace xwalk {

void RuntimeGeolocationPermissionContext::
CancelGeolocationPermissionRequestOnUIThread(
    content::WebContents* web_contents,
    const GURL& requesting_frame) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

#if defined(OS_ANDROID)
  XWalkContent* xwalk_content =
      XWalkContent::FromWebContents(web_contents);
  if (xwalk_content) {
      xwalk_content->HideGeolocationPrompt(requesting_frame);
  }
#endif
  // TODO(yongsheng): Handle this for other platforms.
}

void RuntimeGeolocationPermissionContext::CancelGeolocationPermissionRequest(
    content::WebContents* web_contents,
    const GURL& requesting_frame) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &RuntimeGeolocationPermissionContext::
              CancelGeolocationPermissionRequestOnUIThread,
          this,
          web_contents,
          requesting_frame));
}

RuntimeGeolocationPermissionContext::~RuntimeGeolocationPermissionContext() {
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermissionOnUIThread(
    content::WebContents* web_contents,
    const GURL& requesting_frame,
    base::Callback<void(bool)> result_callback) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

#if defined(OS_ANDROID)
  XWalkContent* xwalk_content =
      XWalkContent::FromWebContents(web_contents);
  if (!xwalk_content) {
    result_callback.Run(false);
    return;
  }

  xwalk_content->ShowGeolocationPrompt(requesting_frame, result_callback);
#elif defined(OS_TIZEN)
  int render_view_id = web_contents->GetRenderViewHost()->GetRoutingID();
  bool has_geolocation_permission = false;
  XWalkRunner* runner = XWalkRunner::GetInstance();
  application::ApplicationSystem* app_system = runner->app_system();
  application::ApplicationService* app_service =
      app_system->application_service();
  application::Application* application =
      app_service->GetApplicationByRenderHostID(render_view_id);

  if (application) {
    DCHECK(application->data());
    application::PermissionsInfo* info =
      static_cast<application::PermissionsInfo*>(
      application->data()->GetManifestData(
          application_manifest_keys::kPermissionsKey));

    if (info) {
      const application::PermissionSet& permissions = info->GetAPIPermissions();
      application::PermissionSet::const_iterator it =
          std::find(permissions.begin(), permissions.end(), "geolocation");
      has_geolocation_permission = it != permissions.end();
    }
  }

  result_callback.Run(has_geolocation_permission);
#endif

  // TODO(yongsheng): Handle this for other platforms.
}

void
RuntimeGeolocationPermissionContext::RequestGeolocationPermission(
    content::WebContents* web_contents,
    const GURL& requesting_frame,
    base::Callback<void(bool)> result_callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(
          &RuntimeGeolocationPermissionContext::
              RequestGeolocationPermissionOnUIThread,
          this,
          web_contents,
          requesting_frame,
          result_callback));
}

}  // namespace xwalk
