// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_

#include <memory>

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

  XWalkExtensionServer* in_process_ui_thread_server() const {
    return in_process_ui_thread_server_.get();
  }

  std::unique_ptr<XWalkExtensionProcessHost> extension_process_host() {
    return std::move(extension_process_host_);
  }

  content::RenderProcessHost* render_process_host() const {
    return render_process_host_;
  }

  ExtensionServerMessageFilter* in_process_message_filter() const {
    return in_process_message_filter_;
  }

  void set_in_process_extension_thread_server(
      std::unique_ptr<XWalkExtensionServer> server) {
    in_process_extension_thread_server_.reset(server.release());
  }

  void set_in_process_ui_thread_server(
      std::unique_ptr<XWalkExtensionServer> server) {
    in_process_ui_thread_server_.reset(server.release());
  }

  void set_extension_process_host(std::unique_ptr<XWalkExtensionProcessHost> host) {
    extension_process_host_.reset(host.release());
  }

  void set_extension_thread(base::Thread* thread) {
    extension_thread_ = thread;
  }

  void set_render_process_host(content::RenderProcessHost* rph) {
    render_process_host_ = rph;
  }

  void set_in_process_message_filter(ExtensionServerMessageFilter* filter) {
    in_process_message_filter_ = filter;
  }

 private:
  // Extension servers living on their respective threads.
  std::unique_ptr<XWalkExtensionServer> in_process_extension_thread_server_;
  std::unique_ptr<XWalkExtensionServer> in_process_ui_thread_server_;

  // This object lives on the IO-thread.
  std::unique_ptr<XWalkExtensionProcessHost> extension_process_host_;

  base::Thread* extension_thread_;

  content::RenderProcessHost* render_process_host_;
  ExtensionServerMessageFilter* in_process_message_filter_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_DATA_H_
