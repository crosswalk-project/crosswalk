// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_PEPPER_PEPPER_HELPER_H_
#define XWALK_RUNTIME_RENDERER_PEPPER_PEPPER_HELPER_H_

#include "base/compiler_specific.h"
#include "content/public/renderer/render_frame_observer.h"

// This class listens for Pepper creation events from the RenderFrame and
// attaches the parts required for Chrome-specific plugin support.
class PepperHelper : public content::RenderFrameObserver {
 public:
  explicit PepperHelper(content::RenderFrame* render_frame);
  virtual ~PepperHelper();

  // RenderFrameObserver.
  virtual void DidCreatePepperPlugin(content::RendererPpapiHost* host) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(PepperHelper);
};

#endif  // XWALK_RUNTIME_RENDERER_PEPPER_PEPPER_HELPER_H_
