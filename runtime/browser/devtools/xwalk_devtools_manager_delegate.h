// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_MANAGER_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_MANAGER_DELEGATE_H_

#include "base/compiler_specific.h"
#include "components/devtools_http_handler/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_manager_delegate.h"

namespace devtools_http_handler {
class DevToolsHttpHandler;
}

namespace xwalk {

class XWalkBrowserContext;

class XWalkDevToolsManagerDelegate : public content::DevToolsManagerDelegate {
 public:
  static devtools_http_handler::DevToolsHttpHandler* CreateHttpHandler(
      XWalkBrowserContext* browser_context);

  ~XWalkDevToolsManagerDelegate() override;

  // DevToolsManagerDelegate implementation.
  void Inspect(content::BrowserContext* browser_context,
               content::DevToolsAgentHost* agent_host) override {}
  void DevToolsAgentStateChanged(content::DevToolsAgentHost* agent_host,
                                 bool attached) override {}
  base::DictionaryValue* HandleCommand(content::DevToolsAgentHost* agent_host,
                                       base::DictionaryValue* command) override;

 private:

  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsManagerDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_DEVTOOLS_XWALK_DEVTOOLS_MANAGER_DELEGATE_H_
