// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/binding/browser/binding_process_host.h"

#include "base/command_line.h"
#include "base/process_util.h"
#include "build/build_config.h"
#include "content/browser/browser_child_process_host_impl.h"
#include "content/common/child_process.h"
#include "content/public/common/child_process_host.h"
#include "content/public/common/content_switches.h"
#include "ipc/ipc_switches.h"
#include "xwalk/binding/binding/binding_thread.h"
#include "xwalk/binding/common/binding_messages.h"
#include "xwalk/binding/common/binding_switches.h"

#if defined(OS_WIN)
#include "content/public/common/sandboxed_process_launcher_delegate.h"
#endif

namespace xwalk {

#if defined(OS_WIN)
class BindingSandboxedProcessLauncherDelegate
    : public content::SandboxedProcessLauncherDelegate {
 public:
  BindingSandboxedProcessLauncherDelegate() {}
  virtual ~BindingSandboxedProcessLauncherDelegate() {}

  virtual void ShouldSandbox(bool* in_sandbox) OVERRIDE {
    *in_sandbox = false;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(BindingSandboxedProcessLauncherDelegate);
};
#endif  // defined(OS_WIN)

class BindingMainThread : public base::Thread {
 public:
  explicit BindingMainThread(const std::string& channel_id)
      : base::Thread("XWalk_InProcBindingThread"),
        channel_id_(channel_id) {
  }

  virtual ~BindingMainThread() {
    Stop();
  }

 protected:
  virtual void Init() OVERRIDE {
    if (!content::ChildProcess::current())
      binding_process_.reset(new content::ChildProcess());
    binding_thread_.reset(new BindingThread(channel_id_));
    if (binding_process_)
      binding_process_->set_main_thread(binding_thread_.release());
  }

  virtual void CleanUp() OVERRIDE {
    binding_process_.reset();
    binding_thread_.reset();
  }

 private:
  std::string channel_id_;
  scoped_ptr<content::ChildProcess> binding_process_;
  scoped_ptr<BindingThread> binding_thread_;

  DISALLOW_COPY_AND_ASSIGN(BindingMainThread);
};


BindingProcessHost* BindingProcessHost::Get() {
  static BindingProcessHost* host = NULL;
  if (host)  return host;
  host = new BindingProcessHost();
  if (host->Init())
    return host;

  delete host;
  return NULL;
}

#define PROCESS_TYPE_BINDING (content::PROCESS_TYPE_CONTENT_END + 1)
BindingProcessHost::BindingProcessHost() {
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kSingleProcess) ||
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kBindingInProcess))
    in_process_ = true;

  process_.reset(
      new content::BrowserChildProcessHostImpl(PROCESS_TYPE_BINDING, this));
}

BindingProcessHost::~BindingProcessHost() {
  std::string message;
  if (!in_process_) {
    int exit_code;
    base::TerminationStatus status = process_->GetTerminationStatus(&exit_code);
    switch (status) {
      case base::TERMINATION_STATUS_NORMAL_TERMINATION:
        message = "The binding process exited normally. Everything is okay.";
        break;
      case base::TERMINATION_STATUS_ABNORMAL_TERMINATION:
        message = base::StringPrintf(
            "The binding process exited with code %d.",
            exit_code);
        break;
      case base::TERMINATION_STATUS_PROCESS_WAS_KILLED:
        message = "You killed the binding process! Why?";
        break;
      case base::TERMINATION_STATUS_PROCESS_CRASHED:
        message = "The binding process crashed!";
        break;
      default:
        break;
    }
  }
}

bool BindingProcessHost::Init() {
  std::string channel_id = process_->GetHost()->CreateChannel();
  if (channel_id.empty())
    return false;

  if (!in_process_)
    return LaunchBindingProcess(channel_id);

  in_process_binding_thread_.reset(new BindingMainThread(channel_id));
  in_process_binding_thread_->Start();
  OnProcessLaunched();  // Fake a callback that the process is ready.
  return true;
}

bool BindingProcessHost::LaunchBindingProcess(const std::string& channel_id) {
  const CommandLine& browser_command_line = *CommandLine::ForCurrentProcess();
  CommandLine::StringType binding_launcher =
      browser_command_line.GetSwitchValueNative(switches::kBindingLauncher);

#if defined(OS_LINUX)
  int child_flags = binding_launcher.empty() ?
                        content::ChildProcessHost::CHILD_ALLOW_SELF :
                        content::ChildProcessHost::CHILD_NORMAL;
#else
  int child_flags = content::ChildProcessHost::CHILD_NORMAL;
#endif

  base::FilePath exe_path =
      content::ChildProcessHost::GetChildPath(child_flags);
  if (exe_path.empty())
    return false;

  CommandLine* cmd_line = new CommandLine(exe_path);
  cmd_line->AppendSwitchASCII(switches::kProcessType,
                              switches::kBindingProcess);
  cmd_line->AppendSwitchASCII(switches::kProcessChannelID, channel_id);

  // If specified, prepend a launcher program to the command line.
  if (!binding_launcher.empty())
    cmd_line->PrependWrapper(binding_launcher);

  process_->Launch(
#if defined(OS_WIN)
      new BindingSandboxedProcessLauncherDelegate,
#elif defined(OS_POSIX)
      false,
      base::EnvironmentVector(),
#endif
      cmd_line);

  process_->SetTerminateChildOnShutdown(false);

  return true;
}

void BindingProcessHost::ForceShutdown() {
  Send(new BindingProcessMsg_ShuttingDown());
  process_->ForceShutdown();
}

bool BindingProcessHost::Send(IPC::Message* msg) {
  if (process_->GetHost()->IsChannelOpening()) {
    queued_messages_.push(msg);
    return true;
  }

  return process_->Send(msg);
}

void BindingProcessHost::OnChannelConnected(int32 peer_pid) {
  while (!queued_messages_.empty()) {
    Send(queued_messages_.front());
    queued_messages_.pop();
  }
}

void BindingProcessHost::OnChannelError() {
  // FIXME(zliang7) notifiy all binding channel host
}

bool BindingProcessHost::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BindingProcessHost, msg)
    IPC_MESSAGE_HANDLER(BindingProcessHostMsg_ChannelCreated, OnChannelCreated)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  DCHECK(handled);
  return handled;
}

void BindingProcessHost::OnChannelCreated(
    int renderer_id,
    const IPC::ChannelHandle& channel_handle) {
  CallbackMap::iterator it;
  it = callbacks_.find(renderer_id);
  if (it != callbacks_.end()) {
    it->second.Run(channel_handle);
    callbacks_.erase(it);
  }
}

void BindingProcessHost::CreateBindingChannel(
    int renderer_id,
    const GURL& url,
    const std::vector<std::string>& features,
    ChannelCreatedCallback cb) {
  callbacks_[renderer_id] = cb;
  Send(new BindingProcessMsg_CreateChannel(renderer_id, url, features));
}

}  // namespace xwalk
