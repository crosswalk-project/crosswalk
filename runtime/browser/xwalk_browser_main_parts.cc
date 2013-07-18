// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/strings/string_number_conversions.h"
#include "xwalk/experimental/dialog/dialog_extension.h"
#include "xwalk/extensions/browser/xwalk_extension_external.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/runtime/browser/devtools/remote_debugging_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "net/base/net_util.h"

namespace xwalk {

using extensions::XWalkExternalExtension;

XWalkBrowserMainParts::XWalkBrowserMainParts(
    const content::MainFunctionParams& parameters)
    : BrowserMainParts(),
      startup_url_(chrome::kAboutBlankURL),
      parameters_(parameters),
      run_default_message_loop_(true) {
}

XWalkBrowserMainParts::~XWalkBrowserMainParts() {
}

void XWalkBrowserMainParts::PreMainMessageLoopStart() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  const CommandLine::StringVector& args = command_line->GetArgs();

  if (args.empty())
    return;

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme())
    startup_url_ = url;
  else
    startup_url_ = net::FilePathToFileURL(base::FilePath(args[0]));

#if defined(OS_MACOSX)
    PreMainMessageLoopStartMac();
#endif
}

void XWalkBrowserMainParts::PostMainMessageLoopStart() {
#if defined(OS_ANDROID)
  base::MessageLoopForUI::current()->Start();
#endif
}

void XWalkBrowserMainParts::PreEarlyInitialization() {
}

void XWalkBrowserMainParts::RegisterExternalExtensions() {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkExternalExtensionsPath))
    return;

  if (!startup_url_.SchemeIsFile()) {
    VLOG(0) << "Unsupported scheme for external extensions: " <<
          startup_url_.scheme();
    return;
  }

  base::FilePath extensions_dir =
      cmd_line->GetSwitchValuePath(switches::kXWalkExternalExtensionsPath);
  if (!file_util::DirectoryExists(extensions_dir)) {
    LOG(WARNING) << "Ignoring non-existent extension directory: "
                 << extensions_dir.AsUTF8Unsafe();
    return;
  }

  // FIXME(leandro): Use GetNativeLibraryName() to obtain the proper
  // extension for the current platform.
  const base::FilePath::StringType pattern = FILE_PATH_LITERAL("*.so");
  file_util::FileEnumerator libraries(extensions_dir, false,
        file_util::FileEnumerator::FILES, pattern);

  for (base::FilePath extension_path = libraries.Next();
        !extension_path.empty(); extension_path = libraries.Next()) {
    XWalkExternalExtension* extension =
          new XWalkExternalExtension(extension_path);

    if (extension->is_valid()) {
      extension_service_->RegisterExtension(extension);
    } else {
      delete extension;
    }
  }
}

void XWalkBrowserMainParts::PreMainMessageLoopRun() {
  runtime_context_.reset(new RuntimeContext);
  runtime_registry_.reset(new RuntimeRegistry);
  extension_service_.reset(
      new extensions::XWalkExtensionService(runtime_registry_.get()));

  RegisterInternalExtensions();
  RegisterExternalExtensions();

  CommandLine* command_line = CommandLine::ForCurrentProcess();
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

  // The new created Runtime instance will be managed by RuntimeRegistry.
  Runtime::Create(runtime_context_.get(), startup_url_);

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
  runtime_context_.reset();
}

void XWalkBrowserMainParts::RegisterInternalExtensions() {
  extension_service_->RegisterExtension(
      new DialogExtension(runtime_registry_.get()));  // experimental
}

}  // namespace xwalk
