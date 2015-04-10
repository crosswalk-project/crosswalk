// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_ANDROID_XWALK_PERMISSION_CLIENT_H_
#define XWALK_RUNTIME_RENDERER_ANDROID_XWALK_PERMISSION_CLIENT_H_

#include "content/public/renderer/render_frame_observer.h"
#include "third_party/WebKit/public/web/WebContentSettingsClient.h"

namespace xwalk {

// XWalk implementation of blink::WebPermissionClient.
class XWalkPermissionClient : public content::RenderFrameObserver,
                              public blink::WebContentSettingsClient {
 public:
  explicit XWalkPermissionClient(content::RenderFrame* render_view);

 private:
  ~XWalkPermissionClient() override;

  // blink::WebContentSettingsClient implementation.
  bool allowImage(bool enabledPerSettings,
                          const blink::WebURL& imageURL) override;

  DISALLOW_COPY_AND_ASSIGN(XWalkPermissionClient);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_RENDERER_ANDROID_XWALK_PERMISSION_CLIENT_H_
