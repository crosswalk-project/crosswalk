// Copyright 2012 the V8 project authors. All rights reserved.
//
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// This is the XWalk Extensions Shell. It implements a simple javascript shell,
// based on V8, that can load XWalk Extensions for testing purposes.
// It is a single process application which runs with three threads (main, IO
// and v8).
// The overall implementation started upon v8/samples/shell.cc .

#include <unistd.h>
#include <string>
#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_libevent.h"
#include "base/run_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/task_runner_util.h"
#include "base/threading/thread.h"
#include "ipc/ipc_sync_channel.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/extensions/xesh/xesh_v8_runner.h"


using xwalk::extensions::XWalkExtensionServer;

// Specifies which file XESh will use as input.
const char kInputFilePath[] = "input-file";

namespace {

inline void PrintInitialInfo() {
  fprintf(stderr, "\n---- XESh: XWalk Extensions Shell ----");
  fprintf(stderr, "\nCrosswalk Version: %s\nv8 Version: %s\n", XWALK_VERSION,
      XEShV8Runner::GetV8Version());
}

inline void PrintPromptLine() {
  fprintf(stderr, "xesh> ");
}

std::string ReadLine() {
  std::string result;
  static const size_t kBufferSize = 256;
  char buffer[kBufferSize];

  while (fgets(buffer, kBufferSize - 1, stdin) != NULL) {
    result += buffer;
    size_t len = result.length();
    if (len == 0)
      break;
    char end = result[len - 1];
    if (end == '\n' || end == '\0')
      break;
  }

  // Handle ^D (EOF).
  if (feof(stdin))
    exit(0);

  return result;
}

// This class will watch the stdin file descriptor (i.e. stdin) so we know
// when we can read without blocking its thread. It will live on the IO Thread.
// In case where XESh got executed with the parameter --input-file=
// this class will first read this input file, get its content executed by v8
// and then it will start listening to stdin again.
class InputWatcher : public base::MessagePumpLibevent::Watcher {
 public:
  InputWatcher(XEShV8Runner* v8_runner, base::MessageLoop* v8_loop)
    : is_waiting_v8_runner_(false),
      v8_runner_(v8_runner),
      v8_message_loop_(v8_loop) {}

  virtual ~InputWatcher() {}

  // base::MessagePumpLibevent::Watcher implementation.
  void OnFileCanReadWithoutBlocking(int fd) override {
    if (is_waiting_v8_runner_)
      return;

    CallV8ExecuteString(ReadLine());
  }

  void OnFileCanWriteWithoutBlocking(int fd) override {}

  void StartWatching() {
    CommandLine* cmd_line = CommandLine::ForCurrentProcess();

    if (cmd_line->HasSwitch(kInputFilePath)) {
      std::string file_contents;
      ReadFileToString(cmd_line->GetSwitchValuePath(kInputFilePath),
          &file_contents);

      CallV8ExecuteString(file_contents);
    }

    // base::MessageLoop::current() will return the IO thread message loop.
    base::MessageLoopForIO* io_loop =
        static_cast<base::MessageLoopForIO*>(base::MessageLoop::current());
    io_loop->WatchFileDescriptor(STDIN_FILENO, true,
        base::MessageLoopForIO::WATCH_READ, &fd_watcher_, this);
  }

 private:
  void CallV8ExecuteString(std::string statement) {
    is_waiting_v8_runner_ = true;

    PostTaskAndReplyWithResult(v8_message_loop_->message_loop_proxy(),
        FROM_HERE,
        base::Bind(&XEShV8Runner::ExecuteString, base::Unretained(v8_runner_),
            statement),
        base::Bind(&InputWatcher::OnFinishedV8Execution,
            base::Unretained(this)));
  }

  void OnFinishedV8Execution(const std::string& result) {
    is_waiting_v8_runner_ = false;

    fprintf(stderr, "%s\n", result.c_str());
    PrintPromptLine();
  }

  bool is_waiting_v8_runner_;
  XEShV8Runner* v8_runner_;
  base::MessageLoop* v8_message_loop_;
  base::MessagePumpLibevent::FileDescriptorWatcher fd_watcher_;

  DISALLOW_COPY_AND_ASSIGN(InputWatcher);
};

// Creates and manages the lifetime of the native side of XWalkExtension's
// Framework. That means managing XWalkExtensionServer and its IPC-related
// objects.
class ExtensionManager {
 public:
  ExtensionManager()
    : shutdown_event_(false, false) {
  }

  ~ExtensionManager() {
    shutdown_event_.Signal();
  }

  void LoadExtensions() {
    base::FilePath extensions_dir =
        CommandLine::ForCurrentProcess()->GetSwitchValuePath(
            switches::kXWalkExternalExtensionsPath);

    scoped_ptr<base::ValueMap> runtime_variables(new base::ValueMap);
    (*runtime_variables)["app_id"] = new base::StringValue("xesh");

    std::vector<std::string> extensions =
        RegisterExternalExtensionsInDirectory(&server_, extensions_dir,
            runtime_variables.Pass());

    fprintf(stderr, "\nExtensions Loaded:\n");
    std::vector<std::string>::const_iterator it = extensions.begin();
    for (; it != extensions.end(); ++it)
      fprintf(stderr, "- %s\n", it->c_str());
  }

  void Initialize(base::MessageLoopProxy* io_message_loop_proxy) {
    handle_ = IPC::Channel::GenerateVerifiedChannelID(std::string());

    server_channel_ = IPC::SyncChannel::Create(handle_,
        IPC::Channel::MODE_SERVER, &server_, io_message_loop_proxy, true,
        &shutdown_event_);

    server_.Initialize(server_channel_.get());
  }

  const IPC::ChannelHandle& ipc_channel_handle() { return handle_; }

 private:
  IPC::ChannelHandle handle_;
  base::WaitableEvent shutdown_event_;
  XWalkExtensionServer server_;
  scoped_ptr<IPC::SyncChannel> server_channel_;
};
}  // namespace

int main(int argc, char* argv[]) {
  base::AtExitManager exit_manager;
  CommandLine::Init(argc, argv);

  PrintInitialInfo();

  base::MessageLoop main_message_loop(base::MessageLoop::TYPE_UI);
  main_message_loop.set_thread_name("XESh_Main");

  base::Thread io_thread("XESh_IOThread");
  io_thread.StartWithOptions(base::Thread::Options(base::MessageLoop::TYPE_IO,
      0));

  base::Thread v8_thread("XESh_V8Thread");
  v8_thread.StartWithOptions(base::Thread::Options(
      base::MessageLoop::TYPE_DEFAULT, 0));

  ExtensionManager extension_manager;
  extension_manager.LoadExtensions();
  extension_manager.Initialize(io_thread.message_loop_proxy());

  XEShV8Runner v8_runner;
  static_cast<base::MessageLoopForIO*>(v8_thread.message_loop())->PostTask(
      FROM_HERE, base::Bind(&XEShV8Runner::Initialize,
      base::Unretained(&v8_runner), argc, argv, io_thread.message_loop_proxy(),
      extension_manager.ipc_channel_handle()));

  InputWatcher input_watcher(&v8_runner, v8_thread.message_loop());

  static_cast<base::MessageLoopForIO*>(io_thread.message_loop())->PostTask(
      FROM_HERE, base::Bind(&InputWatcher::StartWatching,
      base::Unretained(&input_watcher)));

  PrintPromptLine();
  base::RunLoop run_loop;
  run_loop.Run();

  static_cast<base::MessageLoopForIO*>(v8_thread.message_loop())->PostTask(
      FROM_HERE, base::Bind(&XEShV8Runner::Shutdown,
      base::Unretained(&v8_runner)));

  io_thread.Stop();
  v8_thread.Stop();
  return 0;
}
