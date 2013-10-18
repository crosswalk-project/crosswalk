// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_RENDERER_APPLICATION_DISPATCHER_H_
#define XWALK_APPLICATION_RENDERER_APPLICATION_DISPATCHER_H_

#include "content/public/renderer/render_process_observer.h"
#include "v8/include/v8.h"
#include "xwalk/application/common/application.h"

namespace xwalk {
namespace application {

// Dispatches application control messages sent to the renderer.
class ApplicationDispatcher : public content::RenderProcessObserver {
 public:
  ApplicationDispatcher();
  virtual ~ApplicationDispatcher();

  // RenderProcessObserver implementation:
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  void OnShouldSuspend(int sequence_id);
  void OnSuspend();
  void OnCancelSuspend();

  DISALLOW_COPY_AND_ASSIGN(ApplicationDispatcher);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_RENDERER_APPLICATION_DISPATCHER_H_

