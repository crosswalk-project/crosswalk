// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_

#include <map>
#include <string>
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "xwalk/extensions/common/xwalk_extension_runner.h"

namespace xwalk {
namespace extensions {

class XWalkExtension;

// This manages the threads and contexts for a WebContents. It dispatches
// messages from the render process to the right thread and from them to the
// render process.
class XWalkExtensionWebContentsHandler
    : public content::WebContentsObserver,
      public content::WebContentsUserData<XWalkExtensionWebContentsHandler>,
      public XWalkExtensionRunner::Client {
 public:
  virtual ~XWalkExtensionWebContentsHandler();

  void AttachExtension(XWalkExtension* extension);

 private:
  // XWalkExtensionRunner::Client implementation.
  virtual void HandleMessageFromContext(const XWalkExtensionRunner* runner,
                                        const std::string& msg) OVERRIDE;

  // content::WebContentsObserver implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  void OnPostMessage(const std::string& extension_name, const std::string& msg);
  void OnSendSyncMessage(const std::string& extension_name,
                         const std::string& msg, std::string* result);

  friend class content::WebContentsUserData<XWalkExtensionWebContentsHandler>;
  explicit XWalkExtensionWebContentsHandler(content::WebContents* contents);

  void Destroy();

  typedef std::map<std::string, XWalkExtensionRunner*> RunnerMap;
  RunnerMap runners_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionWebContentsHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_WEB_CONTENTS_HANDLER_H_
