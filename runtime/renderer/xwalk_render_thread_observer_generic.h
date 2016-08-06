// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_XWALK_RENDER_THREAD_OBSERVER_GENERIC_H_
#define XWALK_RUNTIME_RENDERER_XWALK_RENDER_THREAD_OBSERVER_GENERIC_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_vector.h"
#include "base/synchronization/lock.h"
#include "content/public/renderer/render_thread_observer.h"
#include "url/gurl.h"
#include "v8/include/v8.h"
#include "xwalk/application/browser/application_security_policy.h"

namespace blink {
class WebFrame;
}  // namespace blink

namespace xwalk {
struct AccessWhitelistItem;

// FIXME: Using filename "xwalk_render_thread_observer_generic.cc(h)" temporary
// , due to the conflict filename with Android port.
// A RenderViewObserver implementation used for handling XWalkView
// specific render-process wide IPC messages.
class XWalkRenderThreadObserver : public content::RenderThreadObserver {
 public:
  XWalkRenderThreadObserver();
  ~XWalkRenderThreadObserver() override;

  // content::RenderThreadObserver implementation.
  bool OnControlMessageReceived(const IPC::Message& message) override;
  void OnRenderProcessShutdown() override;

  bool IsWarpMode() const {
    return security_mode_ == application::ApplicationSecurityPolicy::WARP;
  }
  bool IsCSPMode() const {
    return security_mode_ == application::ApplicationSecurityPolicy::CSP;
  }

  const GURL& app_url() const { return app_url_; }
  bool CanRequest(const GURL& orig, const GURL& dest) const;

 private:
  void OnSetAccessWhiteList(
      const GURL& source,
      const GURL& dest,
      const std::string& dest_host,
      bool allow_subdomains);
  void OnEnableSecurityMode(
      const GURL& url,
      application::ApplicationSecurityPolicy::SecurityMode mode);
  void AddAccessWhiteListEntry(const GURL& source,
                               const GURL& dest,
                               const std::string& dest_host,
                               bool allow_subdomains);

  bool is_blink_initialized_;
  application::ApplicationSecurityPolicy::SecurityMode security_mode_;
  GURL app_url_;
  ScopedVector<AccessWhitelistItem> access_whitelist_;
  mutable base::Lock lock_;
};
}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_XWALK_RENDER_THREAD_OBSERVER_GENERIC_H_
