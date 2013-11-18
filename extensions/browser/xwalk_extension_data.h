// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_

#include "base/memory/scoped_ptr.h"

namespace xwalk {
namespace extensions {

class ExtensionServerMessageFilter;
class XWalkExtensionProcessHost;
class XWalkExtensionServer;

// For each render process we create an ExtensionData with runtime information
// of extensions associated to that particular render process. It holds pointers
// to the servers running in the browser process and the communication channel
// to the extension process.
struct XWalkExtensionData {
  XWalkExtensionData();
  ~XWalkExtensionData();

  // Extension servers living on their respective threads.
  scoped_ptr<XWalkExtensionServer> in_process_extension_thread_server_;
  scoped_ptr<XWalkExtensionServer> in_process_ui_thread_server_;

  // This object lives on the IO-thread.
  ExtensionServerMessageFilter* in_process_message_filter_;

  // This object lives on the IO-thread.
  scoped_ptr<XWalkExtensionProcessHost> extension_process_host_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_
