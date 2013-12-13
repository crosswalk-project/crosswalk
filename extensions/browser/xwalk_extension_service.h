// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_

#include <stdint.h>
#include <map>
#include <string>
#include <vector>
#include "base/callback_forward.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "xwalk/extensions/browser/xwalk_extension_process_host.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"

namespace content {
class RenderProcessHost;
class WebContents;
}

namespace xwalk {
namespace extensions {

class XWalkExtension;
class XWalkExtensionData;

// This is the entry point for Crosswalk extensions. Its responsible for keeping
// track of the extensions, and enable them on WebContents once they are
// created. It's life time follows the Browser process itself.
class XWalkExtensionService : public content::NotificationObserver,
    public XWalkExtensionProcessHost::Delegate {
 public:
  XWalkExtensionService();
  virtual ~XWalkExtensionService();

  void RegisterExternalExtensionsForPath(const base::FilePath& path);

  // To be called when a new RenderProcessHost is created, will plug the
  // extension system to that render process. See
  // XWalkContentBrowserClient::RenderProcessHostCreated().
  //
  // The vectors contain the extensions to be used for this render process,
  // ownership of these extensions is taken by the XWalkExtensionService. The
  // vectors will be empty after the call.
  void OnRenderProcessHostCreated(
      content::RenderProcessHost* host,
      XWalkExtensionVector* ui_thread_extensions,
      XWalkExtensionVector* extension_thread_extensions);

  // To be called when a RenderProcess died, so we can gracefully shutdown the
  // associated ExtensionProcess. See Runtime::RenderProcessGone() and
  // XWalkContentBrowserClient::RenderProcessHostGone().
  void OnRenderProcessDied(content::RenderProcessHost* host);

  typedef base::Callback<void(XWalkExtensionVector* extensions)>
      CreateExtensionsCallback;

  static void SetCreateExtensionThreadExtensionsCallbackForTesting(
      const CreateExtensionsCallback& callback);
  static void SetCreateUIThreadExtensionsCallbackForTesting(
      const CreateExtensionsCallback& callback);

  static void SetExternalExtensionsPathForTesting(const base::FilePath& path);

 private:
  // XWalkExtensionProcessHost::Delegate implementation.
  virtual void OnExtensionProcessDied(XWalkExtensionProcessHost* eph,
      int render_process_id) OVERRIDE;

  // NotificationObserver implementation.
  virtual void Observe(int type, const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  void OnRenderProcessHostClosed(content::RenderProcessHost* host);

  void CreateInProcessExtensionServers(
      content::RenderProcessHost* host,
      XWalkExtensionData* data,
      XWalkExtensionVector* ui_thread_extensions,
      XWalkExtensionVector* extension_thread_extensions);

  void CreateExtensionProcessHost(content::RenderProcessHost* host,
      XWalkExtensionData* data);

  // The server that handles in process extensions will live in the
  // extension_thread_.
  base::Thread extension_thread_;

  content::NotificationRegistrar registrar_;

  base::FilePath external_extensions_path_;

  typedef std::map<int, XWalkExtensionData*> RenderProcessToExtensionDataMap;
  RenderProcessToExtensionDataMap extension_data_map_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionService);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
