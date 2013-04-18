// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_DEVTOOLS_FRONTEND_H_
#define CAMEO_SRC_BROWSER_SHELL_DEVTOOLS_FRONTEND_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_client_host.h"
#include "content/public/browser/devtools_frontend_host_delegate.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
class RenderViewHost;
class WebContents;
}

namespace cameo {

class Shell;

class ShellDevToolsFrontend : public content::WebContentsObserver,
                              public content::DevToolsFrontendHostDelegate {
 public:
  static ShellDevToolsFrontend* Show(content::WebContents* web_contents);
  void Focus();
  void Close();
  Shell* inspected_shell() const {
    return inspected_shell_;
  }
  void set_inspected_shell(Shell* inspected_shell) {
    inspected_shell_ = inspected_shell;
  }

 private:
  ShellDevToolsFrontend(Shell* frontend_shell,
                        content::DevToolsAgentHost* agent_host);
  virtual ~ShellDevToolsFrontend();

  // WebContentsObserver overrides
  virtual void RenderViewCreated(
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void WebContentsDestroyed(
      content::WebContents* web_contents) OVERRIDE;

  // DevToolsFrontendHostDelegate implementation
  virtual void ActivateWindow() OVERRIDE {}
  virtual void ChangeAttachedWindowHeight(unsigned height) OVERRIDE {}
  virtual void CloseWindow() OVERRIDE {}
  virtual void MoveWindow(int x, int y) OVERRIDE {}
  virtual void SetDockSide(const std::string& side) OVERRIDE {}
  virtual void OpenInNewTab(const std::string& url) OVERRIDE {}
  virtual void SaveToFile(const std::string& url,
                          const std::string& content,
                          bool save_as) OVERRIDE {}
  virtual void AppendToFile(const std::string& url,
                            const std::string& content) OVERRIDE {}
  virtual void RequestFileSystems() OVERRIDE {}
  virtual void AddFileSystem() OVERRIDE {}
  virtual void RemoveFileSystem(const std::string& file_system_path) OVERRIDE {}
  virtual void InspectedContentsClosing() OVERRIDE;

  Shell* inspected_shell_;
  Shell* frontend_shell_;
  scoped_refptr<content::DevToolsAgentHost> agent_host_;
  scoped_ptr<content::DevToolsClientHost> frontend_host_;

  DISALLOW_COPY_AND_ASSIGN(ShellDevToolsFrontend);
};

}  // namespace cameo

#endif  // CAMEO_SRC_BROWSER_SHELL_DEVTOOLS_FRONTEND_H_
