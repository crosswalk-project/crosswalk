// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread.h"
#include "base/timer/timer.h"
#include "dbus/bus.h"
#include "dbus/exported_object.h"
#include "dbus/message.h"
#include "dbus/object_manager.h"
#include "dbus/object_path.h"
#include "dbus/property.h"
#include "xwalk/application/common/constants.h"

namespace xwalk {
namespace application {

// Here -1 means waiting for a D-Bus command to return, forever.
// We can not guarantee how much time a command will take. Some command
// may take much longer time than the others, like Install.
#define TIMEOUT -1
#define P(x) printf("%s\n", (std::string(x)).c_str());

dbus::Bus* session_bus = NULL;
dbus::ObjectProxy* object_proxy = NULL;
base::MessageLoop* main_message_loop = NULL;
std::string running_app_id;

void PrintUsage() {
  P("Usage:");
  P("  --" + std::string(kSwitchLaunch) + "=<AppID>      "
    "Launch an application by ID.");
  P("  --" + std::string(kSwitchInstall) + "=<Path>      "
    "Install package from <path>.");
  P("  --" + std::string(kSwitchUninstall) + "=<AppID>   "
    "Uninstall an application by ID.");
  P("  --" + std::string(kSwitchListApps) + "           "
    "List installed applications.");
  P("  --" + std::string(kSwitchVerbose) + "             "
    "Verbose output.");
}

void OnResponse(bool quit, dbus::Response* response) {
  if (response != NULL) {
    dbus::MessageReader reader(response);
    std::string message;
    bool ret;
    reader.PopBool(&ret);
    reader.PopString(&message);
    if (ret) {
      LOG(INFO) << "Command succeed.";
    } else {
      P("Command failed.");
    }
    P(message);
  } else {
    P("Error in processing D-Bus command.");
    quit = true;
  }
  // Normally, the launcher will not exit after issuing a launch command
  // in order to handle system events and then pass to the daemon.
  // TODO(Bai): Handle system events after the daemon counterpart is ready.
  if (quit) {
    session_bus->ShutdownOnDBusThreadAndBlock();
    base::MessageLoop::current()->QuitWhenIdle();
  }
}

void OnTerminate(void) {
  LOG(INFO) << "Terminating: " + running_app_id;
  dbus::MethodCall method_call(kDbusAppInterfaceName, "Terminate");
  dbus::MessageWriter writer(&method_call);
  writer.AppendString(running_app_id);
  object_proxy->CallMethod(&method_call, TIMEOUT,
                           base::Bind(OnResponse, true));
}

void sig_handler(int signo) {
  if (signo != SIGINT)
    return;

  if (main_message_loop) {
    // Considering the Non-signal safe PostTask, a better approach
    // is needed here, though the race condition should be very rare.
    main_message_loop->PostTask(FROM_HERE, base::Bind(&OnTerminate));
  }
}

int launcher_main(int argc, const char *argv[]) {
  bool run_message_loop = false;
  CommandLine::Init(argc, argv);
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  base::AtExitManager exit_manager;

  // Init Logging
  logging::LoggingSettings loggingSettings;
  loggingSettings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  logging::InitLogging(loggingSettings);
  if (!command_line.HasSwitch(kSwitchVerbose)) {
    logging::SetMinLogLevel(10);
  }

  main_message_loop = new base::MessageLoop(base::MessageLoop::TYPE_IO);
  main_message_loop->set_thread_name("Origin");

  // Init D-Bus.
  base::Thread dbus_thread("D-Bus Client Thread");
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  if (!dbus_thread.StartWithOptions(thread_options)) {
    LOG(ERROR) << "Failed to create D-Bus client thread.";
    return -1;
  }
  dbus::Bus::Options options;
  options.bus_type = dbus::Bus::SESSION;
  options.connection_type = dbus::Bus::PRIVATE;
  options.dbus_task_runner = dbus_thread.message_loop_proxy();
  session_bus = new dbus::Bus(options);
  object_proxy = session_bus->GetObjectProxy(kDbusServiceName,
                           dbus::ObjectPath(kDbusObjectPath));

  if (command_line.HasSwitch(kSwitchInstall)) {
    std::string package_path =
              command_line.GetSwitchValueASCII(kSwitchInstall);
    LOG(INFO) << "Installing: " + package_path;
    dbus::MethodCall method_call(kDbusAppInterfaceName, "Install");
    dbus::MessageWriter writer(&method_call);
    writer.AppendString(package_path);
    object_proxy->CallMethod(&method_call, TIMEOUT,
                             base::Bind(OnResponse, true));
    run_message_loop = true;
  } else if (command_line.HasSwitch(kSwitchUninstall)) {
    std::string id =
              command_line.GetSwitchValueASCII(kSwitchUninstall);
    LOG(INFO) << "Uninstalling: " + id;
    dbus::MethodCall method_call(kDbusAppInterfaceName, "Uninstall");
    dbus::MessageWriter writer(&method_call);
    writer.AppendString(id);
    object_proxy->CallMethod(&method_call, TIMEOUT,
                             base::Bind(OnResponse, true));
    run_message_loop = true;
  } else if (command_line.HasSwitch(kSwitchLaunch)) {
    running_app_id =
              command_line.GetSwitchValueASCII(kSwitchLaunch);
    LOG(INFO) << "Launching: " + running_app_id;
    dbus::MethodCall method_call(kDbusAppInterfaceName, "Launch");
    dbus::MessageWriter writer(&method_call);
    writer.AppendString(running_app_id);
    object_proxy->CallMethod(&method_call, TIMEOUT,
                             base::Bind(OnResponse, false));
    // We won't exit until SIGINT.
    if (signal(SIGINT, sig_handler) == SIG_ERR)
      PLOG(ERROR) << "Can't catch SIGINT: ";
    run_message_loop = true;
  } else if (command_line.HasSwitch(kSwitchListApps)) {
    LOG(INFO) << "Listing Applications.";
    dbus::MethodCall method_call(kDbusAppInterfaceName, "ListApps");
    dbus::MessageWriter writer(&method_call);
    object_proxy->CallMethod(&method_call, TIMEOUT,
                             base::Bind(OnResponse, true));
    run_message_loop = true;
  } else {
    PrintUsage();
  }
  if (run_message_loop)
    main_message_loop->Run();
  delete main_message_loop;
  return 0;
}

}  // namespace application
}  // namespace xwalk

int main(int argc, const char *argv[]) {
    return xwalk::application::launcher_main(argc, argv);
}
