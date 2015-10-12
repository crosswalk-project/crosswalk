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
