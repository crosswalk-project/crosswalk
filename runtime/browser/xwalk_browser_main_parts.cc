// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

#include <stdlib.h>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/extension/application_event_extension.h"
#include "xwalk/application/extension/application_runtime_extension.h"
#include "xwalk/experimental/dialog/dialog_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/runtime/browser/devtools/remote_debugging_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/runtime/extension/runtime_extension.h"
#include "xwalk/sysapps/raw_socket/raw_socket_extension.h"
#include "cc/base/switches.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/result_codes.h"
#include "net/base/net_util.h"
#include "ui/gl/gl_switches.h"

#if defined(USE_AURA) && defined(USE_X11)
#include "ui/base/ime/input_method_initializer.h"
#include "ui/events/x/touch_factory_x11.h"
#endif

namespace {

// FIXME: Compare with method in startup_browser_creator.cc.
GURL GetURLFromCommandLine(const CommandLine& command_line) {
  const CommandLine::StringVector& args = command_line.GetArgs();

  if (args.empty())
    return GURL();

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme())
    return url;

  base::FilePath path(args[0]);
  if (!path.IsAbsolute())
    path = MakeAbsoluteFilePath(path);

  return net::FilePathToFileURL(path);
}

}  // namespace

namespace xswitches {
// Redefine settings not exposed by content module.
const char kEnableViewport[] = "enable-viewport";
const char kEnableOverlayScrollbars[] = "enable-overlay-scrollbars";
}

namespace xwalk {

XWalkBrowserMainParts::XWalkBrowserMainParts(
    const content::MainFunctionParams& parameters)
    : BrowserMainParts(),
      startup_url_(content::kAboutBlankURL),
      parameters_(parameters),
      run_default_message_loop_(true) {
}

XWalkBrowserMainParts::~XWalkBrowserMainParts() {
}

void XWalkBrowserMainParts::PreMainMessageLoopStart() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  command_line->AppendSwitch(xswitches::kEnableViewport);

  command_line->AppendSwitch(xswitches::kEnableOverlayScrollbars);

  // Enable multithreaded GPU compositing of web content.
  // This also enables pinch on Tizen.
  command_line->AppendSwitch(switches::kEnableThreadedCompositing);

  // Show feedback on touch.
  command_line->AppendSwitch(switches::kEnableGestureTapHighlight);

  // FIXME: Add comment why this is needed on Android and Tizen.
  command_line->AppendSwitch(switches::kAllowFileAccessFromFiles);

  startup_url_ = GetURLFromCommandLine(*command_line);
}

void XWalkBrowserMainParts::PostMainMessageLoopStart() {
}

void XWalkBrowserMainParts::PreEarlyInitialization() {
#if defined(USE_AURA) && defined(USE_X11)
    ui::InitializeInputMethodForTesting();
#endif
#if defined(OS_LINUX)
  // FIXME: We disable the setuid sandbox on Linux because we don't ship
  // the setuid binary. It is important to remember that the seccomp-bpf
  // sandbox is still fully operational if supported by the kernel. See
  // issue #496.
  //
  // switches::kDisableSetuidSandbox is not being used here because it
  // doesn't have the CONTENT_EXPORT macro despite the fact it is exposed by
  // content_switches.h.
  CommandLine::ForCurrentProcess()->AppendSwitch("disable-setuid-sandbox");
#endif
}

int XWalkBrowserMainParts::PreCreateThreads() {
  return content::RESULT_CODE_NORMAL_EXIT;
}

void XWalkBrowserMainParts::RegisterExternalExtensions() {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkExternalExtensionsPath))
    return;

  if (!cmd_line->HasSwitch(
          switches::kXWalkAllowExternalExtensionsForRemoteSources) &&
      !startup_url_.SchemeIsFile()) {
    VLOG(0) << "Unsupported scheme for external extensions: " <<
          startup_url_.scheme();
    return;
  }

  base::FilePath extensions_dir =
      cmd_line->GetSwitchValuePath(switches::kXWalkExternalExtensionsPath);
  if (!base::DirectoryExists(extensions_dir)) {
    LOG(WARNING) << "Ignoring non-existent extension directory: "
                 << extensions_dir.AsUTF8Unsafe();
    return;
  }

  extension_service_->RegisterExternalExtensionsForPath(extensions_dir);
}

void XWalkBrowserMainParts::PreMainMessageLoopRun() {
  runtime_context_.reset(new RuntimeContext);
  runtime_registry_.reset(new RuntimeRegistry);

  runtime_registry_->AddObserver(
      runtime_context_->GetApplicationSystem()->process_manager());

  CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (!command_line->HasSwitch(switches::kInstall) &&
      !command_line->HasSwitch(switches::kUninstall)) {
    extension_service_.reset(new extensions::XWalkExtensionService(this));

    RegisterExternalExtensions();
  }

  if (command_line->HasSwitch(switches::kRemoteDebuggingPort)) {
    std::string port_str =
        command_line->GetSwitchValueASCII(switches::kRemoteDebuggingPort);
    int port;
    const char* loopback_ip = "127.0.0.1";
    if (base::StringToInt(port_str, &port) && port > 0 && port < 65535) {
      remote_debugging_server_.reset(
          new RemoteDebuggingServer(runtime_context_.get(),
              loopback_ip, port, std::string()));
    }
  }

  NativeAppWindow::Initialize();

  xwalk::application::ApplicationSystem* app_system =
      runtime_context_->GetApplicationSystem();
  if (app_system->HandleApplicationManagementCommands(*command_line,
                                                      startup_url_)) {
    run_default_message_loop_ = false;
    return;
  }

  if (app_system->is_running_as_service()) {
    // In service mode, Crosswalk doesn't launch anything, just waits
    // for external requests to launch apps.
    VLOG(1) << "Crosswalk running as Service.";
    return;
  }

  if (app_system->LaunchFromCommandLine(*command_line, startup_url_,
                                        &run_default_message_loop_)) {
    return;
  }

  // The new created Runtime instance will be managed by RuntimeRegistry.
  Runtime::CreateWithDefaultWindow(runtime_context_.get(), startup_url_);

  // If the |ui_task| is specified in main function parameter, it indicates
  // that we will run this UI task instead of running the the default main
  // message loop. See |content::BrowserTestBase::SetUp| for |ui_task| usage
  // case.
  if (parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_default_message_loop_ = false;
  }
}

bool XWalkBrowserMainParts::MainMessageLoopRun(int* result_code) {
  return !run_default_message_loop_;
}

void XWalkBrowserMainParts::PostMainMessageLoopRun() {
  runtime_registry_->RemoveObserver(
      runtime_context_->GetApplicationSystem()->process_manager());
  runtime_context_.reset();
}

void XWalkBrowserMainParts::RegisterInternalExtensionsInExtensionThreadServer(
    extensions::XWalkExtensionServer* server) {
  CHECK(server);
  server->RegisterExtension(scoped_ptr<XWalkExtension>(new RuntimeExtension()));
  server->RegisterExtension(scoped_ptr<XWalkExtension>(
      new experimental::DialogExtension(runtime_registry_.get())));
  server->RegisterExtension(scoped_ptr<XWalkExtension>(
      new sysapps::RawSocketExtension()));
}

void XWalkBrowserMainParts::RegisterInternalExtensionsInUIThreadServer(
    extensions::XWalkExtensionServer* server) {
  CHECK(server);
  DCHECK(runtime_context_);
  server->RegisterExtension(scoped_ptr<XWalkExtension>(
      new ApplicationRuntimeExtension(
          runtime_context_->GetApplicationSystem())));
  server->RegisterExtension(scoped_ptr<XWalkExtension>(
      new ApplicationEventExtension(runtime_context_->GetApplicationSystem())));
}

}  // namespace xwalk
