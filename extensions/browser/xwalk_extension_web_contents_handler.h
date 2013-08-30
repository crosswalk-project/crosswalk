// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_

#include <stdint.h>
#include <string>
#include "base/values.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace content {
class RenderProcessHost;
}

namespace xwalk {
namespace extensions {

class XWalkExtension;
class XWalkExtensionRunnerStore;
class XWalkExtensionService;
class XWalkExtensionMessageFilter;

// This manages extension runners for a WebContents, routing IPC messages
// received from the runners and vice-versa. Each WebContents contains many
// frames, so this class holds runners for multiple frames.
class XWalkExtensionWebContentsHandler
    : public content::WebContentsUserData<XWalkExtensionWebContentsHandler>,
      public XWalkExtensionRunner::Client {
 public:
  virtual ~XWalkExtensionWebContentsHandler();

  void AttachExtensionRunner(int64_t frame_id, XWalkExtensionRunner* runner);

  XWalkExtensionRunnerStore* runner_store() { return runners_.get(); }
  int routing_id() { return web_contents_->GetRoutingID(); }

  void DidCreateScriptContext(int64_t frame_id);

 private:
  friend class XWalkExtensionService;

  void set_extension_service(XWalkExtensionService* extension_service) {
    extension_service_ = extension_service;
  }

  void set_render_process_host(content::RenderProcessHost* host);
  void ClearMessageFilter(void);

  // XWalkExtensionRunner::Client implementation.
  virtual void HandleMessageFromNative(const XWalkExtensionRunner* runner,
                                        scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleReplyMessageFromNative(
      scoped_ptr<IPC::Message> ipc_reply, scoped_ptr<base::Value> msg) OVERRIDE;

  friend class content::WebContentsUserData<XWalkExtensionWebContentsHandler>;
  explicit XWalkExtensionWebContentsHandler(content::WebContents* contents);

  content::WebContents* web_contents_;
  content::RenderProcessHost* render_process_host_;

  XWalkExtensionService* extension_service_;
  XWalkExtensionMessageFilter* message_filter_;

  scoped_ptr<XWalkExtensionRunnerStore> runners_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionWebContentsHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_
