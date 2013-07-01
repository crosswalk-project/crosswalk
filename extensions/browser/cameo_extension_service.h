// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_EXTENSIONS_BROWSER_CAMEO_EXTENSION_SERVICE_H_
#define CAMEO_EXTENSIONS_BROWSER_CAMEO_EXTENSION_SERVICE_H_

#include <map>
#include <string>
#include "base/callback_forward.h"
#include "cameo/runtime/browser/runtime_registry.h"

namespace content {
class RenderProcessHost;
class WebContents;
}

namespace cameo {
namespace extensions {

class CameoExtension;
class CameoExtensionWebContentsHandler;

// This is the entry point for Cameo extensions. Its responsible for keeping
// track of the extensions, and enable them on WebContents once they are
// created. It's life time follows the Browser process itself.
class CameoExtensionService : public RuntimeRegistryObserver {
 public:
  explicit CameoExtensionService(RuntimeRegistry* runtime_registry);
  virtual ~CameoExtensionService();

  // Takes |extension| ownership. Returns false if it couldn't be registered
  // because another one with the same name exists, otherwise returns true.
  bool RegisterExtension(CameoExtension* extension);

  // To be called when a new RenderProcessHost is created, will plug the
  // extension system to that render process. See
  // CameoContentBrowserClient::RenderProcessHostCreated().
  void OnRenderProcessHostCreated(content::RenderProcessHost* host);

  CameoExtension* GetExtensionForName(const std::string& name);

  // RuntimeRegistryObserver implementation.
  virtual void OnRuntimeAdded(Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(Runtime* runtime) OVERRIDE {}
  virtual void OnRuntimeAppIconChanged(Runtime* runtime) OVERRIDE {}

  typedef base::Callback<void(CameoExtensionService* extension_service)>
      RegisterExtensionsCallback;
  static void SetRegisterExtensionsCallbackForTesting(
      const RegisterExtensionsCallback& callback);

 private:
  void RegisterExtensionsForNewHost(content::RenderProcessHost* host);

  void CreateWebContentsHandler(content::WebContents* web_contents);

  typedef std::map<std::string, CameoExtension*> ExtensionMap;
  ExtensionMap extensions_;

  RuntimeRegistry* runtime_registry_;

  // FIXME(cmarcelo): For now we support only one render process host.
  content::RenderProcessHost* render_process_host_;

  DISALLOW_COPY_AND_ASSIGN(CameoExtensionService);
};

}  // namespace extensions
}  // namespace cameo

#endif  // CAMEO_EXTENSIONS_BROWSER_CAMEO_EXTENSION_SERVICE_H_
