// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_RENDERER_BINDING_CHANNEL_HOST_H_
#define XWALK_BINDING_RENDERER_BINDING_CHANNEL_HOST_H_

#include "content/common/np_channel_base.h"

namespace content {
struct NPVariant_Param;
}

namespace xwalk {
class IsListeningFilter;

// Encapsulates an IPC channel between the renderer and one plugin process.
// On the plugin side there's a corresponding BindingChannel.
class BindingChannelHost : public content::NPChannelBase {
 public:
  static BindingChannelHost* GetBindingChannelHost(
      const IPC::ChannelHandle& channel_handle,
      base::MessageLoopProxy* ipc_message_loop);

  bool Init(base::MessageLoopProxy* ipc_message_loop,
            bool create_pipe_now,
            base::WaitableEvent* shutdown_event) OVERRIDE;

  int GenerateRouteID() OVERRIDE;

  static void SetListening(bool flag);
  static bool IsListening();

  static void Broadcast(IPC::Message* message) {
    content::NPChannelBase::Broadcast(message);
  }

  bool expecting_shutdown() { return expecting_shutdown_; }

 private:
  // Called on the render thread
  BindingChannelHost();
  virtual ~BindingChannelHost();

  static content::NPChannelBase* ClassFactory() {
    return new BindingChannelHost();
  }

  bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;
  void OnSetException(int route_id, const content::NPVariant_Param& param);
  void OnShuttingDown();

  // An IPC MessageFilter that can be told to filter out all messages. This is
  // used when the JS debugger is attached in order to avoid browser hangs.
  scoped_refptr<IsListeningFilter> is_listening_filter_;

  // True if we are expecting the plugin process to go away - in which case,
  // don't treat it as a crash.
  bool expecting_shutdown_;

  DISALLOW_COPY_AND_ASSIGN(BindingChannelHost);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_RENDERER_BINDING_CHANNEL_HOST_H_
