// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_

#include <stdint.h>
#include <map>
#include <string>
#include "base/callback_forward.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"

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
  class Delegate {
   public:
    virtual void RegisterInternalExtensionsInServer(
        XWalkExtensionServer* server) {}

   protected:
    ~Delegate() {}
  };

  explicit XWalkExtensionService(Delegate* delegate);
  virtual ~XWalkExtensionService();

  void RegisterExternalExtensionsForPath(const base::FilePath& path);

  // To be called when a new RenderProcessHost is created, will plug the
  // extension system to that render process. See
  // XWalkContentBrowserClient::RenderProcessHostCreated().
  void OnRenderProcessHostCreated(content::RenderProcessHost* host);

  typedef base::Callback<void(XWalkExtensionService* extension_service,
      XWalkExtensionServer* server)> RegisterExtensionsCallback;
  static void SetRegisterExtensionsCallbackForTesting(
      const RegisterExtensionsCallback& callback);

 private:
  // NotificationObserver implementation.
  virtual void Observe(int type, const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  void OnRenderProcessHostClosed(content::RenderProcessHost* host);

  scoped_ptr<XWalkExtensionServer> CreateInProcessExtensionServer(
      content::RenderProcessHost* host);
  void CreateExtensionProcessHost(content::RenderProcessHost* host);

  // The server that handles in process extensions will live in the
  // extension_thread_.
  base::Thread extension_thread_;

  // This object lives on the IO-thread.
  typedef std::map<int, ExtensionServerMessageFilter*>
      RenderProcessToServerMessageFilterMap;
  RenderProcessToServerMessageFilterMap in_process_server_message_filters_map_;

  content::NotificationRegistrar registrar_;

  Delegate* delegate_;

  base::FilePath external_extensions_path_;

  // This object lives on the IO-thread.
  typedef base::ScopedPtrHashMap<int, XWalkExtensionProcessHost>
      RenderProcessToExtensionProcessHostMap;
  RenderProcessToExtensionProcessHostMap extension_process_hosts_map_;

  // The servers will live on the extension thread.
  typedef base::ScopedPtrHashMap<int, XWalkExtensionServer>
      RenderProcessToInProcessExtensionServerMap;
  RenderProcessToInProcessExtensionServerMap in_process_extension_server_map_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionService);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
