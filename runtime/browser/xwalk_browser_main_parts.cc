// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"

#include <stdlib.h>

#include <set>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "cc/base/switches.h"
#include "components/devtools_http_handler/devtools_http_handler.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/result_codes.h"
#include "net/base/filename_util.h"
#include "ui/gl/gl_switches.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#if defined(USE_GTK_UI)
#include "xwalk/runtime/browser/ui/gtk2_ui.h"
#endif
#include "xwalk/runtime/browser/devtools/xwalk_devtools_manager_delegate.h"
#include "xwalk/runtime/browser/ui/xwalk_javascript_native_dialog_factory.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if !defined(DISABLE_NACL)
#include "components/nacl/browser/nacl_browser.h"
#include "components/nacl/browser/nacl_process_host.h"
#include "xwalk/runtime/browser/nacl_host/nacl_browser_delegate_impl.h"
#endif

#if defined(USE_AURA) && defined(USE_X11)
#include "ui/events/devices/x11/touch_factory_x11.h"
#endif

#if defined(USE_AURA)
#include "ui/wm/core/wm_state.h"
#endif

#if !defined(OS_CHROMEOS) && defined(USE_AURA) && defined(OS_LINUX)
#include "ui/base/ime/input_method_initializer.h"
#endif

namespace {

// FIXME: Compare with method in startup_browser_creator.cc.
GURL GetURLFromCommandLine(const base::CommandLine& command_line) {
  const base::CommandLine::StringVector& args = command_line.GetArgs();

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
const char kEnableOverlayScrollbars[] = "enable-overlay-scrollbars";
}

namespace xwalk {

XWalkBrowserMainParts::XWalkBrowserMainParts(
    const content::MainFunctionParams& parameters)
    : xwalk_runner_(XWalkRunner::GetInstance()),
      extension_service_(NULL),
      startup_url_(url::kAboutBlankURL),
      parameters_(parameters),
      run_default_message_loop_(true),
      devtools_http_handler_(nullptr) {
#if defined(OS_LINUX)
  // FIXME: We disable the setuid sandbox on Linux because we don't ship
  // the setuid binary. It is important to remember that the seccomp-bpf
  // sandbox is still fully operational if supported by the kernel. See
  // issue #496.
  //
  // switches::kDisableSetuidSandbox is not being used here because it
  // doesn't have the CONTENT_EXPORT macro despite the fact it is exposed by
  // content_switches.h.
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      "disable-setuid-sandbox");
#endif
}

XWalkBrowserMainParts::~XWalkBrowserMainParts() {
  DCHECK(!devtools_http_handler_);
}

void XWalkBrowserMainParts::PreMainMessageLoopStart() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  command_line->AppendSwitch(xswitches::kEnableOverlayScrollbars);

  // Enable multithreaded GPU compositing of web content.
  command_line->AppendSwitch(switches::kEnableThreadedCompositing);

  // FIXME: Add comment why this is needed on Android.
  command_line->AppendSwitch(switches::kAllowFileAccessFromFiles);

  // Enable SIMD.JS API by default.
  std::string js_flags("--harmony-simd");
  if (command_line->HasSwitch(switches::kJavaScriptFlags)) {
    js_flags += " ";
    js_flags +=
        command_line->GetSwitchValueASCII(switches::kJavaScriptFlags);
  }
  command_line->AppendSwitchASCII(switches::kJavaScriptFlags, js_flags);
  startup_url_ = GetURLFromCommandLine(*command_line);
}

void XWalkBrowserMainParts::PostMainMessageLoopStart() {
}

void XWalkBrowserMainParts::PreEarlyInitialization() {
#if !defined(OS_CHROMEOS) && defined(USE_AURA) && defined(OS_LINUX)
  ui::InitializeInputMethodForTesting();
#if defined(USE_GTK_UI)
  views::LinuxUI* gtk2_ui = BuildGtk2UI();
  gtk2_ui->Initialize();
  views::LinuxUI::SetInstance(gtk2_ui);
#endif  // defined(USE_GTK_UI)
#endif  // !defined(OS_CHROMEOS) && defined(USE_AURA) && defined(OS_LINUX)

#if defined(USE_AURA)
  wm_state_.reset(new wm::WMState);
#endif
}

int XWalkBrowserMainParts::PreCreateThreads() {
  return content::RESULT_CODE_NORMAL_EXIT;
}

void XWalkBrowserMainParts::RegisterExternalExtensions() {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkExternalExtensionsPath))
    return;

  if (!cmd_line->HasSwitch(
          switches::kXWalkAllowExternalExtensionsForRemoteSources) &&
      (!startup_url_.is_empty() && !startup_url_.SchemeIsFile())) {
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
  xwalk_runner_->PreMainMessageLoopRun();

  devtools_http_handler_.reset(
      XWalkDevToolsManagerDelegate::CreateHttpHandler(
          xwalk_runner_->browser_context()));

  extension_service_ = xwalk_runner_->extension_service();

  if (extension_service_)
    RegisterExternalExtensions();

#if !defined(DISABLE_NACL)
  NaClBrowserDelegateImpl* delegate = new NaClBrowserDelegateImpl();
  nacl::NaClBrowser::SetDelegate(delegate);

  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(nacl::NaClProcessHost::EarlyStartup));
#endif

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
#if defined (OS_WIN)
  // The manifest or startup URL was not specified, try to automatically load
  // the manifest.json.
  if (startup_url_.is_empty()) {
    base::FilePath app_dir;
    PathService::Get(base::DIR_MODULE, &app_dir);
    DCHECK(!app_dir.empty());
    base::FilePath path(base::UTF8ToUTF16("approot/manifest.json"));
    if (!path.IsAbsolute()) {
      // MakeAbsoluteFilePath is confused in Centennial.
      path = app_dir.Append(path);
    }
    startup_url_ = GURL(net::FilePathToFileURL(path));
  }
#endif
  if (command_line->HasSwitch(switches::kRemoteDebuggingPort)) {
    std::string port_str =
        command_line->GetSwitchValueASCII(switches::kRemoteDebuggingPort);
    int port;
    base::StringToInt(port_str, &port);
    xwalk_runner_->EnableRemoteDebugging(port);
  }

#if !defined(OS_ANDROID)
  if (command_line->HasSwitch(switches::kXWalkDisableSaveFormData))
    xwalk_runner_->browser_context()->set_save_form_data(false);
#endif

  NativeAppWindow::Initialize();

  if (command_line->HasSwitch(switches::kListFeaturesFlags)) {
    XWalkRuntimeFeatures::GetInstance()->DumpFeaturesFlags();
    run_default_message_loop_ = false;
    return;
  }

  application::ApplicationSystem* app_system = xwalk_runner_->app_system();
  run_default_message_loop_ = app_system->LaunchFromCommandLine(
      *command_line, startup_url_);
  // If the |ui_task| is specified in main function parameter, it indicates
  // that we will run this UI task instead of running the the default main
  // message loop. See |content::BrowserTestBase::SetUp| for |ui_task| usage
  // case.
  if (parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_default_message_loop_ = false;
  }
#if defined(USE_AURA)
  InstallXWalkJavaScriptNativeDialogFactory();
#endif
}

bool XWalkBrowserMainParts::MainMessageLoopRun(int* result_code) {
  return !run_default_message_loop_;
}

void XWalkBrowserMainParts::PostMainMessageLoopRun() {
  xwalk_runner_->PostMainMessageLoopRun();
  devtools_http_handler_.reset();
}

void XWalkBrowserMainParts::CreateInternalExtensionsForUIThread(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
}

void XWalkBrowserMainParts::CreateInternalExtensionsForExtensionThread(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
}

}  // namespace xwalk
