// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_in_process_extension_handler.h"

#include "base/message_loop/message_loop_proxy.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/extensions/browser/xwalk_extension_web_contents_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_threaded_runner.h"

namespace xwalk {
namespace extensions {

XWalkInProcessExtensionHandler::XWalkInProcessExtensionHandler() {}

XWalkInProcessExtensionHandler::~XWalkInProcessExtensionHandler() {
  ExtensionMap::iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it)
    delete it->second;
}

bool XWalkInProcessExtensionHandler::RegisterExtension(
    scoped_ptr<XWalkExtension> extension) {
  if (extensions_.find(extension->name()) != extensions_.end()) {
    LOG(WARNING) << "Ignoring extension with name already registered: "
                 << extension->name();
    return false;
  }

  std::string name = extension->name();
  extensions_[name] = extension.release();
  VLOG(1) << "Extension '" << name << "' registered.";
  return true;
}

void XWalkInProcessExtensionHandler::CreateRunnersForHandler(
    XWalkExtensionWebContentsHandler* handler, int64_t frame_id) {
  ExtensionMap::const_iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    XWalkExtensionRunner* runner = new XWalkExtensionThreadedRunner(
        it->second, handler, base::MessageLoopProxy::current());
    handler->AttachExtensionRunner(frame_id, runner);
  }
}

void XWalkInProcessExtensionHandler::RegisterExtensionsForNewHost(
    content::RenderProcessHost* host) {
  ExtensionMap::iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    XWalkExtension* extension = it->second;
    host->Send(new XWalkViewMsg_RegisterExtension(
        extension->name(), extension->GetJavaScriptAPI()));
  }
}

}  // namespace extensions
}  // namespace xwalk
