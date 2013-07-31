// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BINDING_BINDING_THREAD_H_
#define XWALK_BINDING_BINDING_BINDING_THREAD_H_

#include <string>
#include <vector>

#include "content/common/child_thread.h"

namespace xwalk {

class BindingThread : public content::ChildThread {
 public:
  BindingThread();
  explicit BindingThread(const std::string& channel_id);
  virtual ~BindingThread();
  virtual void Shutdown() OVERRIDE;

 private:
  void Init();

  virtual bool OnControlMessageReceived(const IPC::Message& msg) OVERRIDE;

  // Callback for when a channel has been created.
  void OnCreateChannel(int renderer_id, const GURL& url,
                       const std::vector<std::string>& features);
  void OnShuttingDown();

  bool in_process_;

  DISALLOW_COPY_AND_ASSIGN(BindingThread);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BINDING_BINDING_THREAD_H_
