// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_process_host.h"

#include "base/logging.h"
#include "base/files/file_path.h"
#include "content/public/browser/browser_child_process_host.h"
#include "content/public/browser/render_process_host.h"
#include "ipc/ipc_message.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

namespace xwalk {
namespace extensions {

XWalkExtensionProcessHost::XWalkExtensionProcessHost() {
  StartProcess();
}

XWalkExtensionProcessHost::~XWalkExtensionProcessHost() {
  StopProcess();
}

void XWalkExtensionProcessHost::StartProcess() {}

void XWalkExtensionProcessHost::StopProcess() {}

bool XWalkExtensionProcessHost::RegisterExternalExtension(
    const base::FilePath& extension_path) {
  return Send(new XWalkExtensionProcessMsg_RegisterExtension(extension_path));
}

void XWalkExtensionProcessHost::OnRenderProcessHostCreated(
    content::RenderProcessHost* render_process_host) {}

bool XWalkExtensionProcessHost::Send(IPC::Message* msg) { return false; }

bool XWalkExtensionProcessHost::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionProcessHost, message)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionProcessHost::OnProcessCrashed(int exit_code) {
  VLOG(0) << "Process crashed with exit_code=" << exit_code;
}

}  // namespace extensions
}  // namespace xwalk

