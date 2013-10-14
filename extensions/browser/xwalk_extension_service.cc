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

XWalkExtensionService::XWalkExtensionService(XWalkExtensionService::Delegate*
    delegate)
    : extension_thread_("XWalkExtensionThread"),
      delegate_(delegate) {
  registrar_.Add(this, content::NOTIFICATION_RENDERER_PROCESS_TERMINATED,
                 content::NotificationService::AllBrowserContextsAndSources());

  // IO main loop is needed by extensions watching file descriptors events.
  base::Thread::Options options(base::MessageLoop::TYPE_IO, 0);
  extension_thread_.StartWithOptions(options);
}

XWalkExtensionService::~XWalkExtensionService() {
  // This object should have been released and asked to be deleted in the
  // extension thread.
  if (!in_process_extension_server_map_.empty())
    VLOG(1) << "In_process Extension's Server map is not empty";
}

void XWalkExtensionService::RegisterExternalExtensionsForPath(
    const base::FilePath& path) {
  external_extensions_path_ = path;
}

void XWalkExtensionService::OnRenderProcessHostCreated(
    content::RenderProcessHost* host) {
  CHECK(host);

  scoped_ptr<XWalkExtensionServer> in_process_server =
      CreateInProcessExtensionServer(host);

  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkDisableExtensionProcess))
    CreateExtensionProcessHost(host);
  else if (!external_extensions_path_.empty()) {
    RegisterExternalExtensionsInDirectory(in_process_server.get(),
        external_extensions_path_);
  }

  in_process_extension_server_map_.set(host->GetID(), in_process_server.Pass());
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
  // Invalidate the objects in the different threads so they stop posting
  // messages to each other. This is important because we'll schedule the
  // deletion of both objects to their respective threads.
  ExtensionServerMessageFilter* message_filter =
      in_process_server_message_filters_map_[host->GetID()];
  CHECK(message_filter);

  message_filter->Invalidate();

  scoped_ptr<XWalkExtensionServer> in_process_server =
      in_process_extension_server_map_.take_and_erase(host->GetID());
  CHECK(in_process_server);

  in_process_server->Invalidate();

  // This will cause the filter to be deleted in the IO-thread.
  host->GetChannel()->RemoveFilter(message_filter);

  extension_thread_.message_loop()->DeleteSoon(
      FROM_HERE, in_process_server.release());

  scoped_ptr<XWalkExtensionProcessHost> eph =
      extension_process_hosts_map_.take_and_erase(host->GetID());

  if (eph)
    BrowserThread::DeleteSoon(BrowserThread::IO, FROM_HERE, eph.release());
}

scoped_ptr<XWalkExtensionServer>
XWalkExtensionService::CreateInProcessExtensionServer(
    content::RenderProcessHost* host) {
  scoped_ptr<XWalkExtensionServer> in_process_server(new XWalkExtensionServer);

  IPC::ChannelProxy* channel = host->GetChannel();

  ExtensionServerMessageFilter* message_filter =
      new ExtensionServerMessageFilter(extension_thread_.message_loop_proxy(),
                                       in_process_server.get());
  channel->AddFilter(message_filter);
  in_process_server->Initialize(channel);

  // The filter is owned by the IPC channel but we keep a reference to remove
  // it from the Channel later during a RenderProcess shutdown.
  in_process_server_message_filters_map_[host->GetID()] = message_filter;

  delegate_->RegisterInternalExtensionsInServer(in_process_server.get());

  if (!g_register_extensions_callback.is_null())
    g_register_extensions_callback.Run(this, in_process_server.get());

  in_process_server->RegisterExtensionsInRenderProcess();

  return in_process_server.Pass();
}

void XWalkExtensionService::CreateExtensionProcessHost(
    content::RenderProcessHost* host) {
  scoped_ptr<XWalkExtensionProcessHost> eph(new XWalkExtensionProcessHost());

  if (!external_extensions_path_.empty())
    eph->RegisterExternalExtensions(external_extensions_path_);

  eph->OnRenderProcessHostCreated(host);
  extension_process_hosts_map_.set(host->GetID(), eph.Pass());
}

}  // namespace extensions
}  // namespace xwalk
