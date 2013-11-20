// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_
#define XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_

#include <string>

#include "base/values.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_listener.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"

namespace base {
class FilePath;
}

namespace IPC {
class SyncChannel;
}

namespace xwalk {
namespace extensions {

class XWalkExtension;
class XWalkExtensionRunner;


// This class represents the Extension Process itself.
// It not only represents the extension side of the browser <->
// extension process communication channel, but also the extension side
// of the extension <-> render process channel.
// It will be responsible for handling the native side (instances) of
// External extensions through its XWalkExtensionServer.
class XWalkExtensionProcess : public IPC::Listener,
                              public XWalkExtension::PermissionsDelegate {
 public:
  XWalkExtensionProcess();
  virtual ~XWalkExtensionProcess();
  virtual bool CheckAPIAccessControl(std::string extension_name,
      std::string app_id, std::string api_name);
 private:
  // IPC::Listener implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // Handlers for IPC messages from XWalkExtensionProcessHost.
  void OnRegisterExtensions(const base::FilePath& extension_path);

  void CreateBrowserProcessChannel();
  void CreateRenderProcessChannel();

  base::WaitableEvent shutdown_event_;
  base::Thread io_thread_;
  scoped_ptr<IPC::SyncChannel> browser_process_channel_;
  XWalkExtensionServer extensions_server_;
  scoped_ptr<IPC::SyncChannel> render_process_channel_;
  IPC::ChannelHandle rp_channel_handle_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionProcess);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_EXTENSION_PROCESS_XWALK_EXTENSION_PROCESS_H_
