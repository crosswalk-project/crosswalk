// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_service.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/scoped_native_library.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_process_host.h"
#include "xwalk/extensions/browser/xwalk_extension_process_host.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

namespace {

XWalkExtensionService::RegisterExtensionsCallback
g_register_extensions_callback;

}

// This object will be responsible for filtering messages on the
// BrowserProcess <-> RenderProcess channel and getting them to the
// in_process ExtensionServer.
class ExtensionServerMessageFilter : public IPC::ChannelProxy::MessageFilter {
 public:
  explicit ExtensionServerMessageFilter(XWalkExtensionServer* server);

  // IPC::ChannelProxy::MessageFilter Implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  friend class IPC::ChannelProxy::MessageFilter;
  virtual ~ExtensionServerMessageFilter() {}
  XWalkExtensionServer* server_;
};

ExtensionServerMessageFilter::ExtensionServerMessageFilter(
    XWalkExtensionServer* server)
    : server_(server) {
}

bool ExtensionServerMessageFilter::OnMessageReceived(const IPC::Message& msg) {
  return server_->OnMessageReceived(msg);
}


XWalkExtensionService::XWalkExtensionService()
    : render_process_host_(NULL),
      in_process_server_message_filter_(NULL) {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkEnableExtensionProcess))
    extension_process_host_.reset(new XWalkExtensionProcessHost());

  registrar_.Add(this, content::NOTIFICATION_RENDERER_PROCESS_TERMINATED,
                 content::NotificationService::AllBrowserContextsAndSources());

  // These objects are created on the UI-thread but they will live on the
  // IO-thread. Their deletion will happen on the IO-thread.
  in_process_extensions_server_.reset(new XWalkExtensionServer());

  if (!g_register_extensions_callback.is_null())
    g_register_extensions_callback.Run(this);
}

XWalkExtensionService::~XWalkExtensionService() {
}

bool XWalkExtensionService::RegisterExtension(
    scoped_ptr<XWalkExtension> extension) {
  // Note: for now we only support registering new extensions before
  // render process hosts were created.
  CHECK(!render_process_host_);
  return in_process_extensions_server_->RegisterExtension(extension.Pass());
}

void XWalkExtensionService::RegisterExternalExtensionsForPath(
    const base::FilePath& path) {
  RegisterExternalExtensionsInDirectory(
      in_process_extensions_server_.get(), path);

  if (extension_process_host_)
    extension_process_host_->RegisterExternalExtensions(path);
}

void XWalkExtensionService::OnRenderProcessHostCreated(
    content::RenderProcessHost* host) {
  // FIXME(cmarcelo): For now we support only one render process host.
  if (render_process_host_)
    return;

  render_process_host_ = host;

  IPC::ChannelProxy* channel = render_process_host_->GetChannel();

  // The filter is owned by the IPC channel but we keep a reference to remove
  // it from the Channel later during a RenderProcess shutdown.
  in_process_server_message_filter_ =
      new ExtensionServerMessageFilter(in_process_extensions_server_.get());
  channel->AddFilter(in_process_server_message_filter_);
  in_process_extensions_server_->Initialize(channel);

  in_process_extensions_server_->RegisterExtensionsInRenderProcess();

  if (extension_process_host_)
    extension_process_host_->OnRenderProcessHostCreated(host);
}

// static
void XWalkExtensionService::SetRegisterExtensionsCallbackForTesting(
    const RegisterExtensionsCallback& callback) {
  g_register_extensions_callback = callback;
}

// We use this to keep track of the RenderProcess shutdown events.
// This is _very_ important so we can clean up all we need gracefully,
// avoiding invalid IPC steps after the IPC channel is gonne.
void XWalkExtensionService::Observe(int type,
                              const content::NotificationSource& source,
                              const content::NotificationDetails& details) {
  switch (type) {
    case content::NOTIFICATION_RENDERER_PROCESS_TERMINATED:
    case content::NOTIFICATION_RENDERER_PROCESS_CLOSED: {
      content::RenderProcessHost* rph =
          content::Source<content::RenderProcessHost>(source).ptr();
      if (rph == render_process_host_) {
        in_process_extensions_server_->Invalidate();
        render_process_host_->GetChannel()->RemoveFilter(
            in_process_server_message_filter_);
        in_process_server_message_filter_ = NULL;
        BrowserThread::DeleteSoon(BrowserThread::IO, FROM_HERE,
            in_process_extensions_server_.release());

        if (extension_process_host_) {
          BrowserThread::DeleteSoon(BrowserThread::IO, FROM_HERE,
              extension_process_host_.release());
        }
      }
    }
  }
}

}  // namespace extensions
}  // namespace xwalk
