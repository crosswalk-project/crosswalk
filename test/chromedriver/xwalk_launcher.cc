// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/xwalk_launcher.h"

#include <algorithm>
#include <vector>

#include "base/base64.h"
#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/format_macros.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/process/kill.h"
#include "base/process/launch.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/xwalk_android_impl.h"
#include "xwalk/test/chromedriver/chrome/xwalk_desktop_impl.h"
#include "xwalk/test/chromedriver/chrome/xwalk_existing_impl.h"
#include "xwalk/test/chromedriver/chrome/xwalk_finder.h"
#include "xwalk/test/chromedriver/chrome/device_manager.h"
#include "xwalk/test/chromedriver/chrome/devtools_http_client.h"
#include "xwalk/test/chromedriver/chrome/embedded_automation_extension.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/user_data_dir.h"
#include "xwalk/test/chromedriver/chrome/version.h"
#include "xwalk/test/chromedriver/chrome/web_view.h"
#include "xwalk/test/chromedriver/chrome/zip.h"
#include "xwalk/test/chromedriver/net/port_server.h"
#include "xwalk/test/chromedriver/net/url_request_context_getter.h"
#include "crypto/sha2.h"

#if defined(OS_POSIX)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace {

Status PrepareCommandLine(int port,
                          const Capabilities& capabilities,
                          CommandLine* prepared_command,
                          base::ScopedTempDir* user_data_dir,
                          base::ScopedTempDir* extension_dir,
                          std::vector<std::string>* extension_bg_pages) {
  base::FilePath program = capabilities.binary;
  if (program.empty()) {
    if (!FindXwalk(&program))
      return Status(kUnknownError, "cannot find xwalk binary");
  } else if (!base::PathExists(program)) {
    return Status(kUnknownError,
                  base::StringPrintf("no xwalk binary at %" PRFilePath,
                                     program.value().c_str()));
  }
  CommandLine command(program);
  Switches switches;

  switches.SetSwitch("remote-debugging-port", base::IntToString(port));

  for (std::set<std::string>::const_iterator iter =
           capabilities.exclude_switches.begin();
       iter != capabilities.exclude_switches.end();
       ++iter) {
    switches.RemoveSwitch(*iter);
  }
  switches.SetFromSwitches(capabilities.switches);

  switches.AppendToCommandLine(&command);
  command.AppendArg("about:blank");
  *prepared_command = command;
  return Status(kOk);
}

Status WaitForDevToolsAndCheckVersion(
    const NetAddress& address,
    URLRequestContextGetter* context_getter,
    const SyncWebSocketFactory& socket_factory,
    scoped_ptr<DevToolsHttpClient>* user_client) {
  scoped_ptr<DevToolsHttpClient> client(new DevToolsHttpClient(
      address, context_getter, socket_factory));
  base::TimeTicks deadline =
      base::TimeTicks::Now() + base::TimeDelta::FromSeconds(20);
  Status status = client->Init(deadline - base::TimeTicks::Now());
  if (status.IsError())
    return status;
  if (client->build_no() < kMinimumSupportedChromeBuildNo) {
    return Status(kUnknownError, "Chrome version must be >= " +
        GetMinimumSupportedChromeVersion());
  }

  while (base::TimeTicks::Now() < deadline) {
    WebViewsInfo views_info;
    client->GetWebViewsInfo(&views_info);
    for (size_t i = 0; i < views_info.GetSize(); ++i) {
      if (views_info.Get(i).type == WebViewInfo::kPage) {
        *user_client = client.Pass();
        return Status(kOk);
      }
    }
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(50));
  }
  return Status(kUnknownError, "unable to discover open pages");
}

Status LaunchExistingXwalkSession(
    URLRequestContextGetter* context_getter,
    const SyncWebSocketFactory& socket_factory,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<Chrome>* xwalk) {
  Status status(kOk);
  scoped_ptr<DevToolsHttpClient> devtools_client;
  status = WaitForDevToolsAndCheckVersion(
      capabilities.debugger_address, context_getter, socket_factory,
      &devtools_client);
  if (status.IsError()) {
    return Status(kUnknownError, "cannot connect to xwalk at " +
                      capabilities.debugger_address.ToString(),
                  status);
  }
  xwalk->reset(new XwalkExistingImpl(devtools_client.Pass(),
                                       devtools_event_listeners));
  return Status(kOk);
}

Status LaunchDesktopXwalk(
    URLRequestContextGetter* context_getter,
    int port,
    scoped_ptr<PortReservation> port_reservation,
    const SyncWebSocketFactory& socket_factory,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<Chrome>* xwalk) {
  CommandLine command(CommandLine::NO_PROGRAM);
  base::ScopedTempDir user_data_dir;
  base::ScopedTempDir extension_dir;
  std::vector<std::string> extension_bg_pages;
  Status status = PrepareCommandLine(port,
                                     capabilities,
                                     &command,
                                     &user_data_dir,
                                     &extension_dir,
                                     &extension_bg_pages);
  if (status.IsError())
    return status;

  base::LaunchOptions options;

#if !defined(OS_WIN)
  if (!capabilities.log_path.empty())
    options.environ["CHROME_LOG_FILE"] = capabilities.log_path;
  if (capabilities.detach)
    options.new_process_group = true;
#endif

#if defined(OS_POSIX)
  base::FileHandleMappingVector no_stderr;
  int devnull = -1;
  file_util::ScopedFD scoped_devnull(&devnull);
  if (!CommandLine::ForCurrentProcess()->HasSwitch("verbose")) {
    // Redirect stderr to /dev/null, so that Chrome log spew doesn't confuse
    // users.
    devnull = open("/dev/null", O_WRONLY);
    if (devnull == -1)
      return Status(kUnknownError, "couldn't open /dev/null");
    no_stderr.push_back(std::make_pair(devnull, STDERR_FILENO));
    options.fds_to_remap = &no_stderr;
  }
#endif

#if defined(OS_WIN)
  std::string command_string = base::WideToUTF8(command.GetCommandLineString());
#else
  std::string command_string = command.GetCommandLineString();
#endif
  VLOG(0) << "Launching xwalk: " << command_string;
  base::ProcessHandle process;
  if (!base::LaunchProcess(command, options, &process))
    return Status(kUnknownError, "xwalk failed to start");

  scoped_ptr<DevToolsHttpClient> devtools_client;
  status = WaitForDevToolsAndCheckVersion(
      NetAddress(port), context_getter, socket_factory, &devtools_client);

  if (status.IsError()) {
    int exit_code;
    base::TerminationStatus xwalk_status =
        base::GetTerminationStatus(process, &exit_code);
    if (xwalk_status != base::TERMINATION_STATUS_STILL_RUNNING) {
      std::string termination_reason;
      switch (xwalk_status) {
        case base::TERMINATION_STATUS_NORMAL_TERMINATION:
          termination_reason = "exited normally";
          break;
        case base::TERMINATION_STATUS_ABNORMAL_TERMINATION:
          termination_reason = "exited abnormally";
          break;
        case base::TERMINATION_STATUS_PROCESS_WAS_KILLED:
          termination_reason = "was killed";
          break;
        case base::TERMINATION_STATUS_PROCESS_CRASHED:
          termination_reason = "crashed";
          break;
        default:
          termination_reason = "unknown";
          break;
      }
      return Status(kUnknownError,
                    "Xwalk failed to start: " + termination_reason);
    }
    if (!base::KillProcess(process, 0, true)) {
      int exit_code;
      if (base::GetTerminationStatus(process, &exit_code) ==
          base::TERMINATION_STATUS_STILL_RUNNING)
        return Status(kUnknownError, "cannot kill Xwalk", status);
    }
    return status;
  }
  scoped_ptr<XwalkDesktopImpl> xwalk_desktop(
      new XwalkDesktopImpl(devtools_client.Pass(),
                            devtools_event_listeners,
                            port_reservation.Pass(),
                            process,
                            command,
                            &extension_dir));
  *xwalk = xwalk_desktop.Pass();
  return Status(kOk);
}

Status LaunchAndroidXwalk(
    URLRequestContextGetter* context_getter,
    int port,
    scoped_ptr<PortReservation> port_reservation,
    const SyncWebSocketFactory& socket_factory,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    DeviceManager* device_manager,
    scoped_ptr<Chrome>* xwalk) {
    // TODO(Peter Wang): Need to implement Android xwalk.
    (void) context_getter;
    (void) port;
    (void) port_reservation;
    (void) socket_factory;
    (void) capabilities;
    (void) devtools_event_listeners;
    (void) device_manager;
    (void) xwalk;

    return Status(kOk);
  }
}  // namespace

Status LaunchXwalk(
    URLRequestContextGetter* context_getter,
    const SyncWebSocketFactory& socket_factory,
    DeviceManager* device_manager,
    PortServer* port_server,
    PortManager* port_manager,
    const Capabilities& capabilities,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<Chrome>* xwalk) {
  if (capabilities.IsExistingBrowser()) {
    return LaunchExistingXwalkSession(
        context_getter, socket_factory,
        capabilities, devtools_event_listeners, xwalk);
  }

  int port = 0;
  scoped_ptr<PortReservation> port_reservation;
  Status port_status(kOk);
  if (port_server)
    port_status = port_server->ReservePort(&port, &port_reservation);
  else
    port_status = port_manager->ReservePort(&port, &port_reservation);
  if (port_status.IsError())
    return Status(kUnknownError, "cannot reserve port for Xwalk", port_status);

  if (capabilities.IsAndroid()) {
    return LaunchAndroidXwalk(context_getter,
                               port,
                               port_reservation.Pass(),
                               socket_factory,
                               capabilities,
                               devtools_event_listeners,
                               device_manager,
                               xwalk);
  } else {
    return LaunchDesktopXwalk(context_getter,
                               port,
                               port_reservation.Pass(),
                               socket_factory,
                               capabilities,
                               devtools_event_listeners,
                               xwalk);
  }
}

namespace internal {

Status ProcessExtension(const std::string& extension,
                        const base::FilePath& temp_dir,
                        base::FilePath* path) {
  (void) extension;
  (void) temp_dir;
  (void) path;

  return Status(kOk);
}

void UpdateExtensionSwitch(Switches* switches,
                           const char name[],
                           const base::FilePath::StringType& extension) {
  (void) switches;
  (void) name;
  (void) extension;
}

Status ProcessExtensions(const std::vector<std::string>& extensions,
                         const base::FilePath& temp_dir,
                         bool include_automation_extension,
                         Switches* switches) {
  (void) extensions;
  (void) temp_dir;
  (void) include_automation_extension;
  (void) switches;

  return Status(kOk);
}

}  // namespace internal
