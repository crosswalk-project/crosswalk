// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_

#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_child_process_host_delegate.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_channel_proxy.h"

namespace content {
class BrowserChildProcessHost;
class RenderProcessHost;
}

namespace xwalk {
namespace extensions {

// This class represents the browser side of the browser <-> extension process
// communication channel. It has to run some operations in IO thread for
// creating the extra process.
class XWalkExtensionProcessHost
    : public content::BrowserChildProcessHostDelegate {
 public:
  XWalkExtensionProcessHost(content::RenderProcessHost* render_process_host,
                            const base::FilePath& external_extensions_path);
  virtual ~XWalkExtensionProcessHost();

 private:
  class RenderProcessMessageFilter;

  void StartProcess();
  void StopProcess();

  // Handler for message from Render Process host, it is a synchronous message,
  // that will be replied only when the extension process channel is created.
  void OnGetExtensionProcessChannel(scoped_ptr<IPC::Message> reply);

  // content::BrowserChildProcessHostDelegate implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnProcessCrashed(int exit_code) OVERRIDE;
  virtual void OnProcessLaunched() OVERRIDE;

  // Message Handlers.
  void OnRenderChannelCreated(const IPC::ChannelHandle& channel_id);

  void ReplyChannelHandleToRenderProcess();

  scoped_ptr<content::BrowserChildProcessHost> process_;
  IPC::ChannelHandle ep_rp_channel_handle_;
  content::RenderProcessHost* render_process_host_;
  scoped_ptr<IPC::Message> pending_reply_for_render_process_;

  // We use this filter to know when RP asked for the extension process channel.
  // Formally ownership is with the render process channel, we keep the pointer
  // to remove the filter once we reply the message, since this exchange needs
  // to happen only once.
  IPC::ChannelProxy::MessageFilter* render_process_message_filter_;
  base::FilePath external_extensions_path_;

  bool is_extension_process_channel_ready_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
