// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_service.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/scoped_native_library.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_process_host.h"
#include "ipc/ipc_message_macros.h"
#include "xwalk/extensions/browser/xwalk_extension_process_host.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

namespace {

XWalkExtensionService::RegisterExtensionsCallback
g_register_extensions_callback;

}

// This object intercepts messages destined to a XWalkExtensionServer and
// dispatch them to its task runner. A message loop proxy of a thread is a
// task runner. Like other filters, this filter will run in the IO-thread.
//
// In the case of in process extensions, we will pass the task runner of the
// extension thread.
class ExtensionServerMessageFilter : public IPC::ChannelProxy::MessageFilter {
 public:
  ExtensionServerMessageFilter(
      scoped_refptr<base::SequencedTaskRunner> task_runner,
      XWalkExtensionServer* server)
      : task_runner_(task_runner),
        server_(server) {}

  // Tells the filter to stop dispatching messages to the server.
  void Invalidate() {
    base::AutoLock l(lock_);
    task_runner_ = NULL;
    server_ = NULL;
  }

 private:
  virtual ~ExtensionServerMessageFilter() {}

  // IPC::ChannelProxy::MessageFilter implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE {
    if (IPC_MESSAGE_CLASS(message) == XWalkExtensionClientServerMsgStart) {
      base::AutoLock l(lock_);
      if (!server_)
        return false;
      task_runner_->PostTask(
          FROM_HERE,
          base::Bind(
              base::IgnoreResult(&XWalkExtensionServer::OnMessageReceived),
              base::Unretained(server_), message));
      return true;
    }
    return false;
  }

  // This lock is used to protect access to filter members.
  base::Lock lock_;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  XWalkExtensionServer* server_;
};

XWalkExtensionService::XWalkExtensionService()
    : render_process_host_(NULL),
      extension_thread_("XWalkExtensionThread"),
      in_process_server_message_filter_(NULL) {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkDisableExtensionProcess))
    extension_process_host_.reset(new XWalkExtensionProcessHost());

  registrar_.Add(this, content::NOTIFICATION_RENDERER_PROCESS_TERMINATED,
                 content::NotificationService::AllBrowserContextsAndSources());

  extension_thread_.Start();

  // The server is created here but will live on the extension thread.
  in_process_extensions_server_.reset(new XWalkExtensionServer());

  if (!g_register_extensions_callback.is_null())
    g_register_extensions_callback.Run(this);
}

XWalkExtensionService::~XWalkExtensionService() {
  // This object should already be released and asked to be deleted in the
  // extension thread.
  CHECK(!in_process_extensions_server_);
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
  if (extension_process_host_) {
    extension_process_host_->RegisterExternalExtensions(path);
  } else {
    RegisterExternalExtensionsInDirectory(in_process_extensions_server_.get(),
                                          path);
  }
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
      new ExtensionServerMessageFilter(extension_thread_.message_loop_proxy(),
                                       in_process_extensions_server_.get());
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
      OnRenderProcessHostClosed(rph);
    }
  }
}

void XWalkExtensionService::OnRenderProcessHostClosed(
    content::RenderProcessHost* host) {
  // FIXME(cmarcelo): For now we support only one render process host.
  if (host != render_process_host_)
    return;

  // Invalidate the objects in the different threads so they stop posting
  // messages to each other. This is important because we'll schedule the
  // deletion of both objects to their respective threads.
  in_process_server_message_filter_->Invalidate();
  in_process_extensions_server_->Invalidate();

  // This will caused the filter to be deleted in the IO-thread.
  render_process_host_->GetChannel()->RemoveFilter(
      in_process_server_message_filter_);
  extension_thread_.message_loop()->DeleteSoon(
      FROM_HERE, in_process_extensions_server_.release());

  if (extension_process_host_) {
    BrowserThread::DeleteSoon(BrowserThread::IO, FROM_HERE,
                              extension_process_host_.release());
  }
}

}  // namespace extensions
}  // namespace xwalk
