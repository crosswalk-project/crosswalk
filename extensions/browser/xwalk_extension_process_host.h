// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_

#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_child_process_host_delegate.h"
#include "ipc/ipc_channel_handle.h"

namespace base {
class FilePath;
}

namespace content {
class BrowserChildProcessHost;
class RenderProcessHost;
}

namespace IPC {
class Message;
}

namespace xwalk {
namespace extensions {

// This class represents the browser side of the browser <-> extension process
// communication channel. It has to run some operations in IO thread for
// creating the extra process.
class XWalkExtensionProcessHost
    : public content::BrowserChildProcessHostDelegate {
 public:
  XWalkExtensionProcessHost();
  virtual ~XWalkExtensionProcessHost();

  void RegisterExternalExtensions(const base::FilePath& extension_path);

  void OnRenderProcessHostCreated(content::RenderProcessHost* host);

 private:
  void StartProcess();
  void StopProcess();

  // Thread-safe function to send message to the associated extension process.
  void Send(IPC::Message* msg);

  // content::BrowserChildProcessHostDelegate implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnProcessCrashed(int exit_code) OVERRIDE;
  virtual void OnProcessLaunched() OVERRIDE;

  // Message Handlers.
  void OnRenderChannelCreated(const IPC::ChannelHandle& channel_id);

  void SendChannelHandleToRenderProcess();

  scoped_ptr<content::BrowserChildProcessHost> process_;
  IPC::ChannelHandle ep_rp_channel_handle_;
  content::RenderProcessHost* render_process_host_;

  bool is_extension_process_channel_ready_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
