// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_

#include <stdint.h>
#include <map>
#include <string>
#include "base/values.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace xwalk {
namespace extensions {

class RunnerStore;
class XWalkExtension;
class XWalkExtensionService;

// This manages extension runners for a WebContents, routing IPC messages
// received from the runners and vice-versa. Each WebContents contains many
// frames, so this class holds runners for multiple frames.
class XWalkExtensionWebContentsHandler
    : public content::WebContentsObserver,
      public content::WebContentsUserData<XWalkExtensionWebContentsHandler>,
      public XWalkExtensionRunner::Client {
 public:
  virtual ~XWalkExtensionWebContentsHandler();

  void AttachExtensionRunner(int64_t frame_id, XWalkExtensionRunner* runner);

 private:
  friend class XWalkExtensionService;

  void set_extension_service(XWalkExtensionService* extension_service) {
    extension_service_ = extension_service;
  }

  // XWalkExtensionRunner::Client implementation.
  virtual void HandleMessageFromContext(const XWalkExtensionRunner* runner,
                                        scoped_ptr<base::Value> msg) OVERRIDE;

  // content::WebContentsObserver implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // IPC message handlers.
  void OnPostMessage(int64_t frame_id,
                     const std::string& extension_name,
                     const base::ListValue& msg);
  void OnSendSyncMessage(int64_t frame_id,
                         const std::string& extension_name,
                         const base::ListValue& msg, base::ListValue* result);
  void DidCreateScriptContext(int64_t frame_id);
  void WillReleaseScriptContext(int64_t frame_id);

  friend class content::WebContentsUserData<XWalkExtensionWebContentsHandler>;
  explicit XWalkExtensionWebContentsHandler(content::WebContents* contents);

  scoped_ptr<RunnerStore> runners_;
  XWalkExtensionService* extension_service_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionWebContentsHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_
