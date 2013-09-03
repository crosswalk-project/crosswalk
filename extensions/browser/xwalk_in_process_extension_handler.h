// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_IN_PROCESS_EXTENSION_HANDLER_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_IN_PROCESS_EXTENSION_HANDLER_H_

#include <stdint.h>
#include <map>
#include <string>
#include "base/memory/scoped_ptr.h"

namespace content {
class RenderProcessHost;
}

namespace xwalk {
namespace extensions {

class XWalkExtension;

// Manages extensions running in the browser process. Responsible for creating
// the runners and registering information about the extensions with the Render
// Process.
class XWalkInProcessExtensionHandler {
 public:
  XWalkInProcessExtensionHandler();
  ~XWalkInProcessExtensionHandler();

  bool RegisterExtension(scoped_ptr<XWalkExtension> extension);

  void RegisterExtensionsForNewHost(content::RenderProcessHost* host);

 private:
  typedef std::map<std::string, XWalkExtension*> ExtensionMap;
  ExtensionMap extensions_;

  DISALLOW_COPY_AND_ASSIGN(XWalkInProcessExtensionHandler);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_IN_PROCESS_EXTENSION_HANDLER_H_
