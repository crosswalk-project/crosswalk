// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

#include <stdlib.h>
#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/experimental/dialog/dialog_extension.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/runtime/browser/devtools/remote_debugging_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/runtime/extension/runtime_extension.h"
#include "cc/base/switches.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/result_codes.h"
#include "grit/net_resources.h"
#include "net/base/net_util.h"
#include "net/base/net_module.h"
#include "ui/base/layout.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/gl/gl_switches.h"

#if defined(OS_ANDROID)
#include "content/public/browser/android/compositor.h"
#include "net/android/network_change_notifier_factory_android.h"
#include "net/base/network_change_notifier.h"
#include "ui/base/l10n/l10n_util_android.h"
#endif  // defined(OS_ANDROID)

namespace {

base::StringPiece PlatformResourceProvider(int key) {
  if (key == IDR_DIR_HEADER_HTML) {
    base::StringPiece html_data =
        ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
            IDR_DIR_HEADER_HTML);
    return html_data;
  }
  return base::StringPiece();
}

}  // namespace

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

#if defined(OS_ANDROID)
void XWalkBrowserMainParts::SetRuntimeContext(RuntimeContext* context) {
  runtime_context_ = context;
}
#endif

void XWalkBrowserMainParts::PreMainMessageLoopStart() {
#if defined(OS_ANDROID)
  CommandLine::ForCurrentProcess()->AppendSwitch(
      switches::kAllowFileAccessFromFiles);
  // WebGL is disabled by default on Android, explicitly enable it in switches.
  CommandLine::ForCurrentProcess()->AppendSwitch(
      switches::kEnableExperimentalWebGL);
#endif

#if !defined(OS_ANDROID)
  CommandLine* command_line = CommandLine::ForCurrentProcess();
#if defined(OS_TIZEN_MOBILE)
  command_line->AppendSwitch(switches::kFullscreen);
  command_line->AppendSwitch(switches::kAllowFileAccessFromFiles);
  command_line->AppendSwitch(switches::kIgnoreGpuBlacklist);

  const char* gl_name;
  if (file_util::PathExists(base::FilePath("/usr/lib/xwalk/libosmesa.so")))
    gl_name = gfx::kGLImplementationOSMesaName;
  else if (file_util::PathExists(base::FilePath("/usr/lib/libGL.so")))
    gl_name = gfx::kGLImplementationDesktopName;
  else
    gl_name = gfx::kGLImplementationEGLName;
  command_line->AppendSwitchASCII(switches::kUseGL, gl_name);
#endif
  const CommandLine::StringVector& args = command_line->GetArgs();

  if (args.empty())
    return;

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme()) {
    startup_url_ = url;
  } else {
    base::FilePath path(args[0]);
    if (!path.IsAbsolute())
      path = MakeAbsoluteFilePath(path);
    startup_url_ = net::FilePathToFileURL(path);
  }
#endif

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
#if defined(OS_ANDROID)
  net::NetworkChangeNotifier::SetFactory(
      new net::NetworkChangeNotifierFactoryAndroid());

  CommandLine::ForCurrentProcess()->AppendSwitch(
      cc::switches::kCompositeToMailbox);

  // Initialize the Compositor.
  content::Compositor::Initialize();
#endif

#if defined(OS_LINUX)
  // FIXME: Issue 496. We need to explicitly disable sandboxing on Linux while
  // we do not support it (ie. ship the appropriate binary), otherwise we will
  // crash on startup.
  CommandLine::ForCurrentProcess()->AppendSwitch(switches::kNoSandbox);
#endif
}

int XWalkBrowserMainParts::PreCreateThreads() {
#if defined(OS_ANDROID)
  DCHECK(runtime_context_);
  runtime_context_->InitializeBeforeThreadCreation();
#endif
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
  if (!file_util::DirectoryExists(extensions_dir)) {
    LOG(WARNING) << "Ignoring non-existent extension directory: "
                 << extensions_dir.AsUTF8Unsafe();
    return;
  }

  extension_service_->RegisterExternalExtensionsForPath(extensions_dir);
}

void XWalkBrowserMainParts::PreMainMessageLoopRun() {
#if defined(OS_ANDROID)
  net::NetModule::SetResourceProvider(PlatformResourceProvider);
  if (parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_default_message_loop_ = false;
  }

  DCHECK(runtime_context_);
  runtime_context_->PreMainMessageLoopRun();
#else
  runtime_context_.reset(new RuntimeContext);
  runtime_registry_.reset(new RuntimeRegistry);
  extension_service_.reset(
      new extensions::XWalkExtensionService());

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

  std::string command_name =
      command_line->GetProgram().BaseName().MaybeAsASCII();

#if defined(OS_TIZEN_MOBILE)
  // On Tizen, applications are launched by a symbolic link
  // named like the application ID.
  if (startup_url_.SchemeIsFile() || command_name.compare("xwalk") != 0) {
#else
  if (startup_url_.SchemeIsFile()) {
#endif  // OS_TIZEN_MOBILE
    xwalk::application::ApplicationSystem* system =
        runtime_context_->GetApplicationSystem();
    xwalk::application::ApplicationService* service =
        system->application_service();

    if (xwalk::application::Application::IsIDValid(command_name)) {
      run_default_message_loop_ = service->Launch(command_name);
      return;
    }

    const CommandLine::StringVector& args = command_line->GetArgs();
    std::string id;
    if (args.size() > 0)
      id = std::string(args[0].begin(), args[0].end());
    if (xwalk::application::Application::IsIDValid(id)) {
      run_default_message_loop_ = service->Launch(id);
      return;
    }
    base::FilePath path;
    if (!net::FileURLToFilePath(startup_url_, &path))
      return;
    if (command_line->HasSwitch(switches::kInstall)) {
      if (file_util::PathExists(path)) {
        std::string id;
        if (service->Install(path, &id)) {
#if defined(OS_TIZEN_MOBILE)
          // FIXME: We temporary invoke a python script until the same
          // is implemented in C++.
          base::FilePath tizen_install(
              FILE_PATH_LITERAL("/usr/bin/install_into_pkginfo_db.py"));
          if (file_util::PathExists(tizen_install)) {
            LOG(INFO) << "Register package installation in Tizen.";
            std::string data_path = runtime_context_->GetPath().MaybeAsASCII();
            std::string manifest_path = runtime_context_->GetPath()
                .AppendASCII("applications")
                .AppendASCII(id)
                .AppendASCII("manifest.json")
                .MaybeAsASCII();
            std::string cmd = "/usr/bin/env python "
                + tizen_install.MaybeAsASCII()
                + " -i " + manifest_path
                + " -p " + id
                + " -d " + data_path;

            if (std::system(cmd.c_str()) == 0) {
              LOG(INFO) << "Installed successfully on Tizen.";
            } else {
              LOG(ERROR) << "[ERR] An error occurred during"
                            "installation on Tizen.";
              run_default_message_loop_ = false;
              return;
            }
          }
#endif  // OS_TIZEN_MOBILE
          LOG(INFO) << "[OK] Application installed: " << id;
        } else {
          LOG(ERROR) << "[ERR] Application install failure: " << path.value();
        }
      }
      run_default_message_loop_ = false;
      return;
    } else if (file_util::DirectoryExists(path)) {
      run_default_message_loop_ = service->Launch(path);
      return;
    }
  }

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
#endif
}

bool XWalkBrowserMainParts::MainMessageLoopRun(int* result_code) {
  return !run_default_message_loop_;
}

void XWalkBrowserMainParts::PostMainMessageLoopRun() {
#if defined(OS_ANDROID)
  base::MessageLoopForUI::current()->Start();
#else
  runtime_context_.reset();
#endif
}

void XWalkBrowserMainParts::RegisterInternalExtensions() {
  extension_service_->RegisterExtension(scoped_ptr<XWalkExtension>(
      new RuntimeExtension()));
  extension_service_->RegisterExtension(scoped_ptr<XWalkExtension>(
      new experimental::DialogExtension(runtime_registry_.get())));
}

}  // namespace xwalk
