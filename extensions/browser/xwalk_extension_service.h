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
#include "xwalk/extensions/browser/xwalk_extension_process_host.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace content {
class RenderProcessHost;
class WebContents;
}

namespace xwalk {
namespace extensions {

class XWalkExtensionData;
class XWalkExtensionServer;

// This is the entry point for Crosswalk extensions. Its responsible for keeping
// track of the extensions, and enable them on WebContents once they are
// created. It's life time follows the Browser process itself.
class XWalkExtensionService : public content::NotificationObserver,
    public XWalkExtensionProcessHost::Delegate,
    public XWalkExtension::PermissionsDelegate {
 public:
  class Delegate {
   public:
    virtual void RegisterInternalExtensionsInExtensionThreadServer(
        XWalkExtensionServer* server) {}
    virtual void RegisterInternalExtensionsInUIThreadServer(
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

  // To be called when a RenderProcess died, so we can gracefully shutdown the
  // associated ExtensionProcess. See Runtime::RenderProcessGone() and
  // XWalkContentBrowserClient::RenderProcessHostGone().
  void OnRenderProcessDied(content::RenderProcessHost* host);

  typedef base::Callback<void(XWalkExtensionServer* server)>
      RegisterExtensionsCallback;

  static void SetRegisterExtensionThreadExtensionsCallbackForTesting(
      const RegisterExtensionsCallback& callback);
  static void SetRegisterUIThreadExtensionsCallbackForTesting(
      const RegisterExtensionsCallback& callback);

  static void SetExternalExtensionsPathForTesting(const base::FilePath& path);

  virtual bool CheckAPIAccessControl(std::string extension_name,
      std::string app_id, std::string api_name);

 private:
  // XWalkExtensionProcessHost::Delegate implementation.
  virtual void OnExtensionProcessDied(XWalkExtensionProcessHost* eph,
      int render_process_id) OVERRIDE;

  // NotificationObserver implementation.
  virtual void Observe(int type, const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  void OnRenderProcessHostClosed(content::RenderProcessHost* host);

  void CreateInProcessExtensionServers(content::RenderProcessHost* host,
      XWalkExtensionData* data);
  void CreateExtensionProcessHost(content::RenderProcessHost* host,
      XWalkExtensionData* data);

  // The server that handles in process extensions will live in the
  // extension_thread_.
  base::Thread extension_thread_;

  content::NotificationRegistrar registrar_;

  Delegate* delegate_;

  base::FilePath external_extensions_path_;

  typedef std::map<int, XWalkExtensionData*> RenderProcessToExtensionDataMap;
  RenderProcessToExtensionDataMap extension_data_map_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionService);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
