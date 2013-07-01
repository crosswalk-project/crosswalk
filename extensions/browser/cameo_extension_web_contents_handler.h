// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_EXTENSIONS_BROWSER_CAMEO_EXTENSION_WEB_CONTENTS_HANDLER_H_
#define CAMEO_EXTENSIONS_BROWSER_CAMEO_EXTENSION_WEB_CONTENTS_HANDLER_H_

#include <map>
#include <string>
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace cameo {
namespace extensions {

class CameoExtension;
class CameoExtensionRunner;

// This manages the threads and contexts for a WebContents. It dispatches
// messages from the render process to the right thread and from them to the
// render process.
class CameoExtensionWebContentsHandler
    : public content::WebContentsObserver,
      public content::WebContentsUserData<CameoExtensionWebContentsHandler> {
 public:
  virtual ~CameoExtensionWebContentsHandler();

  void AttachExtension(CameoExtension* extension);

  // content::WebContentsObserver implementation.
  bool OnMessageReceived(const IPC::Message& message);

 private:
  void OnPostMessage(const std::string& extension_name, const std::string& msg);

  friend class content::WebContentsUserData<CameoExtensionWebContentsHandler>;
  explicit CameoExtensionWebContentsHandler(content::WebContents* contents);

  void Destroy();

  typedef std::map<std::string, CameoExtensionRunner*> RunnerMap;
  RunnerMap runners_;

  DISALLOW_COPY_AND_ASSIGN(CameoExtensionWebContentsHandler);
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_EXTENSIONS_BROWSER_CAMEO_EXTENSION_WEB_CONTENTS_HANDLER_H_
