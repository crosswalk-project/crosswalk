// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_

#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_child_process_host_delegate.h"

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

class XWalkExtensionProcessHost
    : public content::BrowserChildProcessHostDelegate {
 public:
  XWalkExtensionProcessHost();
  virtual ~XWalkExtensionProcessHost();

  void StartProcess();
  void StopProcess();

  void RegisterExternalExtensions(const base::FilePath& extension_path);

  void OnRenderProcessHostCreated(content::RenderProcessHost* host);

 private:
  // Thread-safe function to send message to the associated extension process.
  void Send(IPC::Message* msg);

  // content::BrowserChildProcessHostDelegate implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnProcessCrashed(int exit_code) OVERRIDE;
  virtual void OnProcessLaunched() OVERRIDE;

  scoped_ptr<content::BrowserChildProcessHost> process_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_PROCESS_HOST_H_
