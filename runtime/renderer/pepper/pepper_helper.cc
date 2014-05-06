// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/pepper/pepper_helper.h"

#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "xwalk/runtime/renderer/pepper/xwalk_renderer_pepper_host_factory.h"

PepperHelper::PepperHelper(content::RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) {
}

PepperHelper::~PepperHelper() {
}

void PepperHelper::DidCreatePepperPlugin(content::RendererPpapiHost* host) {
  host->GetPpapiHost()->AddHostFactoryFilter(
      scoped_ptr<ppapi::host::HostFactory>(
          new XWalkRendererPepperHostFactory(host)));
}
