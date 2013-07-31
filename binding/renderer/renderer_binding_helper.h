// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_RENDERER_RENDERER_BINDING_HELPER_H_
#define XWALK_BINDING_RENDERER_RENDERER_BINDING_HELPER_H_

#include "content/public/renderer/render_view_observer.h"
#include "content/public/renderer/render_view_observer_tracker.h"
#include "xwalk/binding/renderer/binding_channel_host.h"

namespace xwalk {

class BindingChannelHost;

class RendererBindingHelper
    : public content::RenderViewObserver,
      public content::RenderViewObserverTracker<RendererBindingHelper> {
 public:
  explicit RendererBindingHelper(content::RenderView* render_view);
  virtual void DidClearWindowObject(WebKit::WebFrame* frame) OVERRIDE;

 private:
  scoped_refptr<BindingChannelHost> channel_host_;

  DISALLOW_COPY_AND_ASSIGN(RendererBindingHelper);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_RENDERER_RENDERER_BINDING_HELPER_H_
