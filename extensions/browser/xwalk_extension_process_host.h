// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_

#include <memory>
#include <string>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include "content/public/browser/browser_child_process_host_delegate.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_sender.h"
#include "xwalk/extensions/common/xwalk_extension_permission_types.h"

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
    : public content::BrowserChildProcessHostDelegate,
      public IPC::Sender {
 public:
  class Delegate {
   public:
    virtual void OnExtensionProcessDied(XWalkExtensionProcessHost* eph,
        int render_process_id) {}
    virtual void OnExtensionProcessCreated(int render_process_id,
                                           const IPC::ChannelHandle handle) {}
    virtual void OnCheckAPIAccessControl(int render_process_id,
                                         const std::string& extension_name,
                                         const std::string& api_name,
                                         const PermissionCallback& callback) {}
    virtual bool OnRegisterPermissions(int render_process_id,
                                       const std::string& extension_name,
                                       const std::string& perm_table);
    virtual void OnRenderChannelCreated(int render_process_id) {}

   protected:
    ~Delegate() {}
  };

  XWalkExtensionProcessHost(content::RenderProcessHost* render_process_host,
                            const base::FilePath& external_extensions_path,
                            XWalkExtensionProcessHost::Delegate* delegate,
                            std::unique_ptr<base::ValueMap> runtime_variables);
  ~XWalkExtensionProcessHost() override;

  // IPC::Sender implementation
  bool Send(IPC::Message* msg) override;

 private:
  class RenderProcessMessageFilter;

  void StartProcess();
  void StopProcess();

  // Handler for message from Render Process host, it is a synchronous message,
  // that will be replied only when the extension process channel is created.
  void OnGetExtensionProcessChannel(std::unique_ptr<IPC::Message> reply);

  // content::BrowserChildProcessHostDelegate implementation.
  bool OnMessageReceived(const IPC::Message& message) override;
  void OnChannelError() override;
  void OnProcessLaunched() override;

  // Message Handlers.
  void OnRenderChannelCreated(const IPC::ChannelHandle& channel_id);

  void ReplyChannelHandleToRenderProcess();

  void OnCheckAPIAccessControl(const std::string& extension_name,
      const std::string& api_name, IPC::Message* reply_msg);
  void ReplyAccessControlToExtension(IPC::Message* reply_msg,
      RuntimePermission perm);
  void OnRegisterPermissions(const std::string& extension_name,
      const std::string& perm_table, bool* result);

  std::unique_ptr<content::BrowserChildProcessHost> process_;
  IPC::ChannelHandle ep_rp_channel_handle_;
  content::RenderProcessHost* render_process_host_;
  std::unique_ptr<IPC::Message> pending_reply_for_render_process_;

  // We use this filter to know when RP asked for the extension process channel.
  // We keep the reference to invalidate the filter once we don't need it
  // anymore.
  //
  // TODO(cmarcelo): Avoid having an extra filter, see if we can embed this
  // handling in the existing filter we have in ExtensionData struct.
  scoped_refptr<RenderProcessMessageFilter> render_process_message_filter_;

  base::FilePath external_extensions_path_;

  bool is_extension_process_channel_ready_;

  XWalkExtensionProcessHost::Delegate* delegate_;

  std::unique_ptr<base::ValueMap> runtime_variables_;

  // IPC channel for launcher to communicate with BP in service mode.
  std::unique_ptr<IPC::Channel> channel_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
