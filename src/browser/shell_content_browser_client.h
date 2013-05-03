// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_SHELL_CONTENT_BROWSER_CLIENT_H_
#define CONTENT_SHELL_SHELL_CONTENT_BROWSER_CLIENT_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace content {

class ShellBrowserContext;
class ShellBrowserMainParts;
class ShellResourceDispatcherHostDelegate;

class ShellContentBrowserClient : public ContentBrowserClient,
                                  public NotificationObserver {
 public:
  // Gets the current instance.
  static ShellContentBrowserClient* Get();

  ShellContentBrowserClient();
  virtual ~ShellContentBrowserClient();

  // ContentBrowserClient overrides.
  virtual BrowserMainParts* CreateBrowserMainParts(
      const MainFunctionParams& parameters) OVERRIDE;
  virtual void RenderProcessHostCreated(RenderProcessHost* host) OVERRIDE;
  virtual net::URLRequestContextGetter* CreateRequestContext(
      BrowserContext* browser_context,
      ProtocolHandlerMap* protocol_handlers) OVERRIDE;
  virtual net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      BrowserContext* browser_context,
      const base::FilePath& partition_path,
      bool in_memory,
      ProtocolHandlerMap* protocol_handlers) OVERRIDE;
  virtual void AppendExtraCommandLineSwitches(CommandLine* command_line,
                                              int child_process_id) OVERRIDE;
  virtual void OverrideWebkitPrefs(RenderViewHost* render_view_host,
                                   const GURL& url,
                                   webkit_glue::WebPreferences* prefs) OVERRIDE;
  virtual void ResourceDispatcherHostCreated() OVERRIDE;
  virtual AccessTokenStore* CreateAccessTokenStore() OVERRIDE;
  virtual std::string GetDefaultDownloadName() OVERRIDE;
  virtual bool SupportsBrowserPlugin(content::BrowserContext* browser_context,
                                     const GURL& url) OVERRIDE;
  virtual WebContentsViewDelegate* GetWebContentsViewDelegate(
      WebContents* web_contents) OVERRIDE;
  virtual QuotaPermissionContext* CreateQuotaPermissionContext() OVERRIDE;

#if defined(OS_ANDROID)
  virtual void GetAdditionalMappedFilesForChildProcess(
      const CommandLine& command_line,
      int child_process_id,
      std::vector<content::FileDescriptorInfo>* mappings) OVERRIDE;
#endif

  // NotificationObserver implementation.
  virtual void Observe(int type,
                       const NotificationSource& source,
                       const NotificationDetails& details) OVERRIDE;

  ShellBrowserContext* browser_context();
  ShellBrowserContext* off_the_record_browser_context();
  ShellResourceDispatcherHostDelegate* resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
  ShellBrowserMainParts* shell_browser_main_parts() {
    return shell_browser_main_parts_;
  }

 private:
  ShellBrowserContext* ShellBrowserContextForBrowserContext(
      BrowserContext* content_browser_context);

  scoped_ptr<ShellResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  base::FilePath webkit_source_dir_;

  base::PlatformFile hyphen_dictionary_file_;

  ShellBrowserMainParts* shell_browser_main_parts_;

  NotificationRegistrar registrar_;
};

}  // namespace content

#endif  // CONTENT_SHELL_SHELL_CONTENT_BROWSER_CLIENT_H_
