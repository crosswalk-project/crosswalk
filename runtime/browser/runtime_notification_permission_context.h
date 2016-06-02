// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_NOTIFICATION_PERMISSION_CONTEXT_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_NOTIFICATION_PERMISSION_CONTEXT_H_

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

class RuntimeNotificationPermissionContext
  : public base::RefCountedThreadSafe<RuntimeNotificationPermissionContext> {
 public:
  virtual void RequestNotificationPermission(
      content::WebContents* web_contents,
      const GURL& requesting_frame,
      const std::string& application_name,
      base::Callback<void(bool)> result_callback);
  virtual void CancelNotificationPermissionRequest(
      content::WebContents* web_contents,
      const GURL& requesting_frame);

 protected:
  virtual ~RuntimeNotificationPermissionContext();
  friend class base::RefCountedThreadSafe<RuntimeNotificationPermissionContext>;

private:
#if !defined(OS_ANDROID)
  void OnPermissionRequestFinished(base::Callback<void(bool)>, bool success);
#endif
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_NOTIFICATION_PERMISSION_CONTEXT_H_
