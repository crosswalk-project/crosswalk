// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_

#include <stdint.h>
#include <string>
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

namespace base {
class FilePath;
}

namespace content {
class RenderProcessHost;
class WebContents;
}

namespace xwalk {
namespace extensions {

class ExtensionServerMessageFilter;
class XWalkExtension;
class XWalkExtensionProcessHost;
class XWalkExtensionServer;

// This is the entry point for Crosswalk extensions. Its responsible for keeping
// track of the extensions, and enable them on WebContents once they are
// created. It's life time follows the Browser process itself.
class XWalkExtensionService : public content::NotificationObserver {
 public:
  XWalkExtensionService();
  virtual ~XWalkExtensionService();

  // Returns false if it couldn't be registered because another one with the
  // same name exists, otherwise returns true.
  bool RegisterExtension(scoped_ptr<XWalkExtension> extension);

  void RegisterExternalExtensionsForPath(const base::FilePath& path);

  // To be called when a new RenderProcessHost is created, will plug the
  // extension system to that render process. See
  // XWalkContentBrowserClient::RenderProcessHostCreated().
  void OnRenderProcessHostCreated(content::RenderProcessHost* host);

  typedef base::Callback<void(XWalkExtensionService* extension_service)>
      RegisterExtensionsCallback;
  static void SetRegisterExtensionsCallbackForTesting(
      const RegisterExtensionsCallback& callback);

 private:
  // NotificationObserver implementation.
  virtual void Observe(int type, const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  void OnRenderProcessHostClosed(content::RenderProcessHost* host);

  // FIXME(cmarcelo): For now we support only one render process host.
  content::RenderProcessHost* render_process_host_;

  // The server that handles in process extensions will live in the
  // extension_thread_.
  base::Thread extension_thread_;
  scoped_ptr<XWalkExtensionServer> in_process_extensions_server_;

  // This object lives on the IO-thread.
  ExtensionServerMessageFilter* in_process_server_message_filter_;

  // This object lives on the IO-thread.
  scoped_ptr<XWalkExtensionProcessHost> extension_process_host_;

  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionService);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
