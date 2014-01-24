// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_service.h"

#include <set>
#include <vector>
#include "base/callback.h"
#include "base/command_line.h"
#include "base/pickle.h"
#include "base/scoped_native_library.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_process_host.h"
#include "ipc/ipc_message_macros.h"
#include "xwalk/extensions/browser/xwalk_extension_data.h"
#include "xwalk/extensions/browser/xwalk_extension_process_host.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"

using content::BrowserThread;

namespace xwalk {
namespace extensions {

namespace {

XWalkExtensionService::CreateExtensionsCallback
    g_create_extension_thread_extensions_callback;

XWalkExtensionService::CreateExtensionsCallback
    g_create_ui_thread_extensions_callback;

base::FilePath g_external_extensions_path_for_testing_;

}  // namespace

// This object intercepts messages destined to a XWalkExtensionServer and
// dispatch them to its task runner. A message loop proxy of a thread is a
// task runner. Like other filters, this filter will run in the IO-thread.
//
// In the case of in process extensions, we will pass the task runner of the
// extension thread.
class ExtensionServerMessageFilter : public IPC::ChannelProxy::MessageFilter,
                                     public IPC::Sender {
 public:
  ExtensionServerMessageFilter(
      scoped_refptr<base::SequencedTaskRunner> task_runner,
      XWalkExtensionServer* extension_thread_server,
      XWalkExtensionServer* ui_thread_server)
      : sender_(NULL),
        task_runner_(task_runner),
        extension_thread_server_(extension_thread_server),
        ui_thread_server_(ui_thread_server) {}

  // Tells the filter to stop dispatching messages to the server.
  void Invalidate() {
    base::AutoLock l(lock_);
    sender_ = NULL;
    task_runner_ = NULL;
    extension_thread_server_ = NULL;
    ui_thread_server_ = NULL;
  }

  // IPC::Sender implementation.
  virtual bool Send(IPC::Message* msg_ptr) OVERRIDE {
    scoped_ptr<IPC::Message> msg(msg_ptr);

    if (!sender_)
      return false;

    return sender_->Send(msg.release());
  }

 private:
  virtual ~ExtensionServerMessageFilter() {}

  int64_t GetInstanceIDFromMessage(const IPC::Message& message) {
    PickleIterator iter;

    if (message.is_sync())
      iter = IPC::SyncMessage::GetDataIterator(&message);
    else
      iter = PickleIterator(message);

    int64_t instance_id;
    if (!iter.ReadInt64(&instance_id))
      return -1;

    return instance_id;
  }

  void RouteMessageToServer(const IPC::Message& message) {
    int64_t id = GetInstanceIDFromMessage(message);
    DCHECK_NE(id, -1);

    XWalkExtensionServer* server;
    base::TaskRunner* task_runner;
    scoped_refptr<base::TaskRunner> task_runner_ref;

    if (ContainsKey(extension_thread_instances_ids_, id)) {
      server = extension_thread_server_;
      task_runner = task_runner_;
    } else {
      server = ui_thread_server_;
      task_runner_ref =
          BrowserThread::GetMessageLoopProxyForThread(BrowserThread::UI);
      task_runner = task_runner_ref.get();
    }

    base::Closure closure = base::Bind(
        base::IgnoreResult(&XWalkExtensionServer::OnMessageReceived),
        base::Unretained(server), message);

    task_runner->PostTask(FROM_HERE, closure);
  }

  void OnCreateInstance(int64_t instance_id, std::string name) {
    XWalkExtensionServer* server;
    base::TaskRunner* task_runner;
    scoped_refptr<base::TaskRunner> task_runner_ref;

    if (extension_thread_server_->ContainsExtension(name)) {
      extension_thread_instances_ids_.insert(instance_id);
      server = extension_thread_server_;
      task_runner = task_runner_;
    } else {
      server = ui_thread_server_;
      task_runner_ref =
          BrowserThread::GetMessageLoopProxyForThread(BrowserThread::UI);
      task_runner = task_runner_ref.get();
    }

    base::Closure closure = base::Bind(
        base::IgnoreResult(&XWalkExtensionServer::OnCreateInstance),
        base::Unretained(server), instance_id, name);

    task_runner->PostTask(FROM_HERE, closure);
  }

  void OnGetExtensions(
      std::vector<XWalkExtensionServerMsg_ExtensionRegisterParams>* reply) {
    extension_thread_server_->OnGetExtensions(reply);
    ui_thread_server_->OnGetExtensions(reply);
  }

  // IPC::ChannelProxy::MessageFilter implementation.
  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE {
    sender_ = channel;
  }

  virtual void OnFilterRemoved() OVERRIDE {
    sender_ = NULL;
  }

  virtual void OnChannelClosing() OVERRIDE {
    sender_ = NULL;
  }

  virtual void OnChannelError() OVERRIDE {
    sender_ = NULL;
  }

  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE {
    if (IPC_MESSAGE_CLASS(message) != XWalkExtensionClientServerMsgStart)
      return false;

    base::AutoLock l(lock_);

    if (!extension_thread_server_ || !ui_thread_server_)
      return false;

    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ExtensionServerMessageFilter, message)
      IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_CreateInstance,
                          OnCreateInstance)
      IPC_MESSAGE_HANDLER(XWalkExtensionServerMsg_GetExtensions,
                          OnGetExtensions)
      IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()

    if (!handled)
      RouteMessageToServer(message);

    return true;
  }

  // This lock is used to protect access to filter members.
  base::Lock lock_;

  IPC::Sender* sender_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  XWalkExtensionServer* extension_thread_server_;
  XWalkExtensionServer* ui_thread_server_;
  std::set<int64_t> extension_thread_instances_ids_;
};

XWalkExtensionService::XWalkExtensionService()
    : extension_thread_("XWalkExtensionThread") {
  if (!g_external_extensions_path_for_testing_.empty())
    external_extensions_path_ = g_external_extensions_path_for_testing_;
  registrar_.Add(this, content::NOTIFICATION_RENDERER_PROCESS_TERMINATED,
                 content::NotificationService::AllBrowserContextsAndSources());

  // IO main loop is needed by extensions watching file descriptors events.
  base::Thread::Options options(base::MessageLoop::TYPE_IO, 0);
  extension_thread_.StartWithOptions(options);
}

XWalkExtensionService::~XWalkExtensionService() {
  // This object should have been released and asked to be deleted in the
  // extension thread.
  if (!extension_data_map_.empty())
    VLOG(1) << "The ExtensionData map is not empty!";
}

void XWalkExtensionService::RegisterExternalExtensionsForPath(
    const base::FilePath& path) {
  external_extensions_path_ = path;
}

void XWalkExtensionService::OnRenderProcessHostCreated(
    content::RenderProcessHost* host,
    XWalkExtensionVector* ui_thread_extensions,
    XWalkExtensionVector* extension_thread_extensions,
    const base::ValueMap& runtime_variables) {
  CHECK(host);

  XWalkExtensionData* data = new XWalkExtensionData;
  data->set_render_process_host(host);

  CreateInProcessExtensionServers(host, data, ui_thread_extensions,
                                  extension_thread_extensions);

  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkDisableExtensionProcess))
    CreateExtensionProcessHost(host, data, runtime_variables);
  else if (!external_extensions_path_.empty()) {
    RegisterExternalExtensionsInDirectory(
        data->in_process_ui_thread_server(),
        external_extensions_path_, runtime_variables);
  }

  extension_data_map_[host->GetID()] = data;
}

// static
void
XWalkExtensionService::SetCreateExtensionThreadExtensionsCallbackForTesting(
    const CreateExtensionsCallback& callback) {
  g_create_extension_thread_extensions_callback = callback;
}

// static
void
XWalkExtensionService::SetCreateUIThreadExtensionsCallbackForTesting(
    const CreateExtensionsCallback& callback) {
  g_create_ui_thread_extensions_callback = callback;
}

// static
void XWalkExtensionService::SetExternalExtensionsPathForTesting(
    const base::FilePath& path) {
  g_external_extensions_path_for_testing_ = path;
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
  RenderProcessToExtensionDataMap::iterator it =
      extension_data_map_.find(host->GetID());

  if (it == extension_data_map_.end())
    return;

  XWalkExtensionData* data = it->second;

  // Invalidate the objects in the different threads so they stop posting
  // messages to each other. This is important because we'll schedule the
  // deletion of both objects to their respective threads.
  ExtensionServerMessageFilter* message_filter =
      data->in_process_message_filter();
  CHECK(message_filter);

  message_filter->Invalidate();

  // This will cause the filter to be deleted in the IO-thread.
  host->GetChannel()->RemoveFilter(message_filter);

  extension_data_map_.erase(it);
  delete data;
}

namespace {

void RegisterExtensionsIntoServer(XWalkExtensionVector* extensions,
                                  XWalkExtensionServer* server) {
  XWalkExtensionVector::iterator it = extensions->begin();
  for (; it != extensions->end(); ++it) {
    std::string name = (*it)->name();
    if (!server->RegisterExtension(scoped_ptr<XWalkExtension>(*it))) {
      LOG(WARNING) << "Couldn't register extension with name '"
                   << name << "'\n";
    }
  }
  extensions->clear();
}

}  // namespace


void XWalkExtensionService::CreateInProcessExtensionServers(
    content::RenderProcessHost* host, XWalkExtensionData* data,
    XWalkExtensionVector* ui_thread_extensions,
    XWalkExtensionVector* extension_thread_extensions) {
  scoped_ptr<XWalkExtensionServer> extension_thread_server(
      new XWalkExtensionServer);
  scoped_ptr<XWalkExtensionServer> ui_thread_server(
      new XWalkExtensionServer);

  IPC::ChannelProxy* channel = host->GetChannel();

  extension_thread_server->Initialize(channel);
  ui_thread_server->Initialize(channel);

  RegisterExtensionsIntoServer(extension_thread_extensions,
                               extension_thread_server.get());
  RegisterExtensionsIntoServer(ui_thread_extensions, ui_thread_server.get());

  if (!g_create_ui_thread_extensions_callback.is_null()) {
    XWalkExtensionVector extensions;
    g_create_ui_thread_extensions_callback.Run(&extensions);
    RegisterExtensionsIntoServer(&extensions, ui_thread_server.get());
  }

  if (!g_create_extension_thread_extensions_callback.is_null()) {
    XWalkExtensionVector extensions;
    g_create_extension_thread_extensions_callback.Run(&extensions);
    RegisterExtensionsIntoServer(&extensions, extension_thread_server.get());
  }

  ExtensionServerMessageFilter* message_filter =
      new ExtensionServerMessageFilter(extension_thread_.message_loop_proxy(),
                                       extension_thread_server.get(),
                                       ui_thread_server.get());

  // The filter is owned by the IPC channel but we keep a reference to remove
  // it from the Channel later during a RenderProcess shutdown.
  data->set_in_process_message_filter(message_filter);
  channel->AddFilter(message_filter);

  data->set_in_process_extension_thread_server(extension_thread_server.Pass());
  data->set_in_process_ui_thread_server(ui_thread_server.Pass());

  data->set_extension_thread(&extension_thread_);
}

void XWalkExtensionService::CreateExtensionProcessHost(
    content::RenderProcessHost* host, XWalkExtensionData* data,
    const base::ValueMap& runtime_variables) {
  data->set_extension_process_host(make_scoped_ptr(
      new XWalkExtensionProcessHost(host, external_extensions_path_, this,
                                    runtime_variables)));
}

void XWalkExtensionService::OnExtensionProcessDied(
    XWalkExtensionProcessHost* eph, int render_process_id) {
  // When this is called it means that XWalkExtensionProcessHost is about
  // to be deleted. We should invalidate our reference to it so we avoid a
  // segfault when trying to delete it within
  // XWalkExtensionService::OnRenderProcessHostClosed();

  RenderProcessToExtensionDataMap::iterator it =
      extension_data_map_.find(render_process_id);

  if (it == extension_data_map_.end())
    return;

  XWalkExtensionData* data = it->second;

  XWalkExtensionProcessHost* stored_eph =
      data->extension_process_host().release();
  CHECK_EQ(stored_eph, eph);

  content::RenderProcessHost* rph = data->render_process_host();
  if (rph) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(
        base::IgnoreResult(&content::RenderProcessHost::FastShutdownIfPossible),
        base::Unretained(rph)));
  }

  extension_data_map_.erase(it);
  delete data;
}

void XWalkExtensionService::OnRenderProcessDied(
    content::RenderProcessHost* host) {
  RenderProcessToExtensionDataMap::iterator it =
      extension_data_map_.find(host->GetID());

  if (it == extension_data_map_.end())
    return;

  XWalkExtensionData* data = it->second;

  extension_data_map_.erase(it);
  delete data;
}

}  // namespace extensions
}  // namespace xwalk
