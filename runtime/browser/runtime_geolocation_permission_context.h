// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_GEOLOCATION_PERMISSION_CONTEXT_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_GEOLOCATION_PERMISSION_CONTEXT_H_

#include <string>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "third_party/WebKit/public/platform/modules/permissions/permission_status.mojom.h"

class GURL;

namespace content {
class WebContents;
}

namespace xwalk {

class XWalkBrowserContext;

class RuntimeGeolocationPermissionContext
    : public base::RefCountedThreadSafe<RuntimeGeolocationPermissionContext> {
 public:
  // device::GeolocationPermissionContext implementation.
  virtual void RequestGeolocationPermission(
      content::WebContents* web_contents,
      const GURL& requesting_frame,
      const std::string& application_name,
      base::Callback<void(bool)> result_callback);
  virtual void CancelGeolocationPermissionRequest(
      content::WebContents* web_contents,
      const GURL& requesting_frame);

 protected:
  virtual ~RuntimeGeolocationPermissionContext();
  friend class base::RefCountedThreadSafe<RuntimeGeolocationPermissionContext>;

 private:
#if !defined(OS_ANDROID)
  void OnPermissionRequestFinished(base::Callback<void(bool)>, bool success);
#else
  void RequestGeolocationPermissionOnUIThread(
      content::WebContents* web_contents,
      const GURL& requesting_frame,
      base::Callback<void(bool)> result_callback);

  void CancelGeolocationPermissionRequestOnUIThread(
      content::WebContents* web_contents,
      const GURL& requesting_frame);
#endif
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_GEOLOCATION_PERMISSION_CONTEXT_H_
