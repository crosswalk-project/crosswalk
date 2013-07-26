// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_BROWSER_BINDING_PROCESS_HOST_H_
#define XWALK_BINDING_BROWSER_BINDING_PROCESS_HOST_H_

#include <map>
#include <queue>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_child_process_host_delegate.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_sender.h"

namespace content {
class BrowserChildProcessHostImpl;
}
namespace IPC {
struct ChannelHandle;
}
class GURL;

namespace xwalk {

class BindingMainThread;

class BindingProcessHost : public content::BrowserChildProcessHostDelegate,
                          public IPC::Sender {
 public:
  BindingProcessHost();
  virtual ~BindingProcessHost();

  static BindingProcessHost* Get();

  virtual bool Send(IPC::Message* message) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE;
  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
  virtual void OnChannelError() OVERRIDE;
  virtual bool Init();

  typedef base::Callback<void(const IPC::ChannelHandle&)>
      ChannelCreatedCallback;
  void CreateBindingChannel(int renderer_id,
                            const GURL& url,
                            const std::vector<std::string>& features,
                            ChannelCreatedCallback cb);
  void ForceShutdown();

 private:
  void OnChannelCreated(int renderer_id,
                        const IPC::ChannelHandle& channel_handle);
  bool LaunchBindingProcess(const std::string& channel_id);

  bool in_process_;
  scoped_ptr<BindingMainThread> in_process_binding_thread_;
  scoped_ptr<content::BrowserChildProcessHostImpl> process_;
  std::queue<IPC::Message*> queued_messages_;
  typedef std::map<int, ChannelCreatedCallback> CallbackMap;
  CallbackMap callbacks_;

  DISALLOW_COPY_AND_ASSIGN(BindingProcessHost);
};

}  // namespace xwalk

#endif  // XWALK_BINDING_BROWSER_BINDING_PROCESS_HOST_H_
