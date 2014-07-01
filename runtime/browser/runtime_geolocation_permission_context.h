// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_GEOLOCATION_PERMISSION_CONTEXT_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_GEOLOCATION_PERMISSION_CONTEXT_H_

#include "base/callback.h"
#include "base/memory/ref_counted.h"

class GURL;

namespace xwalk {

class RuntimeContext;

class RuntimeGeolocationPermissionContext
    : public base::RefCountedThreadSafe<RuntimeGeolocationPermissionContext> {
 public:
  // content::GeolocationPermissionContext implementation.
  virtual void RequestGeolocationPermission(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame,
      bool user_gesture,
      base::Callback<void(bool)> callback);
  virtual void CancelGeolocationPermissionRequest(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame);

 protected:
  virtual ~RuntimeGeolocationPermissionContext();
  friend class base::RefCountedThreadSafe<RuntimeGeolocationPermissionContext>;

 private:
  void RequestGeolocationPermissionOnUIThread(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame,
      const base::Callback<void(bool)> callback);

  void CancelGeolocationPermissionRequestOnUIThread(
      int render_process_id,
      int render_view_id,
      int bridge_id,
      const GURL& requesting_frame);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_GEOLOCATION_PERMISSION_CONTEXT_H_
