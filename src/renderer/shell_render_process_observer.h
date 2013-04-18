// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RENDERER_SHELL_RENDER_PROCESS_OBSERVER_H_
#define CAMEO_SRC_RENDERER_SHELL_RENDER_PROCESS_OBSERVER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/render_process_observer.h"

namespace WebKit {
class WebFrame;
}

namespace content {
class RenderView;
}

namespace cameo {

class ShellRenderProcessObserver : public content::RenderProcessObserver {
 public:
  static ShellRenderProcessObserver* GetInstance();

  ShellRenderProcessObserver();
  virtual ~ShellRenderProcessObserver();

  // RenderProcessObserver implementation.
  virtual void WebKitInitialized() OVERRIDE;
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  // Message handlers.

  DISALLOW_COPY_AND_ASSIGN(ShellRenderProcessObserver);
};

}  // namespace cameo

#endif  // CAMEO_SRC_RENDERER_SHELL_RENDER_PROCESS_OBSERVER_H_