// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_PROCESS_OBSERVER_H_
#define XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_PROCESS_OBSERVER_H_

#include <string>

#include "content/public/renderer/render_process_observer.h"

#include "base/compiler_specific.h"

namespace xwalk {

// A RenderProcessObserver implementation used for handling XWalkView
// specific render-process wide IPC messages.
class XWalkRenderProcessObserver : public content::RenderProcessObserver {
 public:
  XWalkRenderProcessObserver();
  ~XWalkRenderProcessObserver() override;

  // content::RenderProcessObserver implementation.
  bool OnControlMessageReceived(const IPC::Message& message) override;

 private:
  void OnSetJsOnlineProperty(bool network_up);
  void OnClearCache();
  void OnSetOriginAccessWhitelist(std::string base_url,
                                  std::string match_patterns);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_ANDROID_XWALK_RENDER_PROCESS_OBSERVER_H_
