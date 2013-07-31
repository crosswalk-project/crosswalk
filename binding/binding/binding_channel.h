// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BINDING_BINDING_CHANNEL_H_
#define XWALK_BINDING_BINDING_BINDING_CHANNEL_H_

#include <string>
#include <vector>

#include "build/build_config.h"
#include "content/common/np_channel_base.h"

class GURL;

namespace xwalk {

// Encapsulates an IPC channel between the binding process and one renderer
// process.  On the renderer side there's a corresponding BindingChannelHost.
class BindingChannel : public content::NPChannelBase {
 public:
  // Get a new BindingChannel object for the current process to talk to the
  // given renderer process. The renderer ID is an opaque unique ID generated
  // by the browser.
  static BindingChannel* GetBindingChannel(
      int renderer_id, base::MessageLoopProxy* ipc_message_loop);

  // Send a message to all renderers that the process is going to shutdown.
  static void NotifyRenderersOfPendingShutdown();

  int GenerateRouteID() OVERRIDE;

#if defined(OS_POSIX)
  int TakeRendererFileDescriptor() {
    return channel_->TakeClientFileDescriptor();
  }
#endif

  void SetFeatures(const std::vector<std::string>& features) {
    features_ = features;
  }
  void SetRootObject(const GURL& url);
  NPObject* GetRootObject() const { return window_npobject_; }

 protected:
  ~BindingChannel();

 private:
  // Called on the binding thread
  BindingChannel();

  bool OnControlMessageReceived(const IPC::Message& msg) OVERRIDE;

  static content::NPChannelBase* ClassFactory() {
    return new BindingChannel();
  }

  void OnGenerateRouteID(int* route_id);
  void OnBindAPIs(bool* result);

  // The id of the renderer who is on the other side of the channel.
  int renderer_id_;

  std::vector<std::string> features_;
  NPObject* window_npobject_;

  DISALLOW_COPY_AND_ASSIGN(BindingChannel);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BINDING_BINDING_CHANNEL_H_
