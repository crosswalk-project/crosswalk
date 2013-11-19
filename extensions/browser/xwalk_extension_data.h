// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_

#include "base/memory/scoped_ptr.h"

namespace base {
class Thread;
}

namespace content {
class RenderProcessHost;
}

namespace xwalk {
namespace extensions {

class ExtensionServerMessageFilter;
class XWalkExtensionProcessHost;
class XWalkExtensionServer;

// For each render process we create an ExtensionData with runtime information
// of extensions associated to that particular render process. It holds pointers
// to the servers running in the browser process and the communication channel
// to the extension process.
class XWalkExtensionData {
 public:
  XWalkExtensionData();
  ~XWalkExtensionData();

  XWalkExtensionServer* in_process_ui_thread_server() {
    return in_process_ui_thread_server_.get();
  }

  ExtensionServerMessageFilter* in_process_message_filter() {
    return in_process_message_filter_;
  }

  scoped_ptr<XWalkExtensionProcessHost> extension_process_host() {
    return extension_process_host_.Pass();
  }

  content::RenderProcessHost* render_process_host() {
    return render_process_host_;
  }

  void set_in_process_extension_thread_server(
      scoped_ptr<XWalkExtensionServer> server) {
    in_process_extension_thread_server_.reset(server.release());
  }

  void set_in_process_ui_thread_server(
      scoped_ptr<XWalkExtensionServer> server) {
    in_process_ui_thread_server_.reset(server.release());
  }

  // We don't take the ownership of the filter because filters are owned by
  // the IPC Channel they are filtering.
  void set_in_process_message_filter(ExtensionServerMessageFilter* filter) {
    in_process_message_filter_ = filter;
  }

  void set_extension_process_host(scoped_ptr<XWalkExtensionProcessHost> host) {
    extension_process_host_.reset(host.release());
  }

  void set_extension_thread(base::Thread* thread) {
    extension_thread_ = thread;
  }

  void set_render_process_host(content::RenderProcessHost* rph) {
    render_process_host_ = rph;
  }

 private:
  // Extension servers living on their respective threads.
  scoped_ptr<XWalkExtensionServer> in_process_extension_thread_server_;
  scoped_ptr<XWalkExtensionServer> in_process_ui_thread_server_;

  // This object lives on the IO-thread.
  ExtensionServerMessageFilter* in_process_message_filter_;

  // This object lives on the IO-thread.
  scoped_ptr<XWalkExtensionProcessHost> extension_process_host_;

  base::Thread* extension_thread_;

  content::RenderProcessHost* render_process_host_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_
