// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"

#include <string>

#include "base/android/path_utils.h"
#include "base/base_paths_android.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/threading/sequenced_worker_pool.h"
#include "cc/base/switches.h"
#include "content/public/browser/android/compositor.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/result_codes.h"
#include "media/base/media_switches.h"
#include "net/android/network_change_notifier_factory_android.h"
#include "net/base/network_change_notifier.h"
#include "net/base/net_module.h"
#include "net/grit/net_resources.h"
#include "ui/base/layout.h"
#include "ui/base/l10n/l10n_util_android.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/runtime/browser/android/cookie_manager.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace {

const char kPreKitkatDataDirectory[] = "app_database";
const char kKitkatDataDirectory[] = "app_webview";

void ImportKitkatDataIfNecessary(const base::FilePath& old_data_dir,
                                 const base::FilePath& profile) {
  if (!base::DirectoryExists(old_data_dir))
    return;

  const char* possible_data_dir_names[] = {
      "Cache",
      "Cookies",
      "Cookies-journal",
      "IndexedDB",
      "Local Storage",
      "databases",
  };
  for (size_t i = 0; i < arraysize(possible_data_dir_names); i++) {
    base::FilePath dir = old_data_dir.Append(possible_data_dir_names[i]);
    if (base::PathExists(dir)) {
      if (!base::Move(dir, profile.Append(possible_data_dir_names[i]))) {
        NOTREACHED() << "Failed to import previous user data: "
                     << possible_data_dir_names[i];
      }
    }
  }
}

void ImportPreKitkatDataIfNecessary(const base::FilePath& old_data_dir,
                                    const base::FilePath& profile) {
  if (!base::DirectoryExists(old_data_dir))
    return;

  // Local Storage.
  base::FilePath local_storage_path = old_data_dir.Append("localstorage");
  if (base::PathExists(local_storage_path)) {
    if (!base::Move(local_storage_path, profile.Append("Local Storage"))) {
      NOTREACHED() << "Failed to import previous user data: localstorage";
    }
  }
}

void MoveUserDataDirIfNecessary(const base::FilePath& user_data_dir,
                                const base::FilePath& profile) {
  if (base::DirectoryExists(profile))
    return;

  if (!base::CreateDirectory(profile))
    return;

  // Import pre-crosswalk-8 data.
  ImportKitkatDataIfNecessary(user_data_dir, profile);
  // Import Android Kitkat System webview data.
  base::FilePath old_data_dir = user_data_dir.DirName().Append(
      kKitkatDataDirectory);
  ImportKitkatDataIfNecessary(old_data_dir, profile);
  // Import pre-Kitkat System webview data.
  old_data_dir = user_data_dir.DirName().Append(kPreKitkatDataDirectory);
  ImportPreKitkatDataIfNecessary(old_data_dir, profile);
}

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

using content::BrowserThread;
using extensions::XWalkExtension;

void GetUserDataDir(base::FilePath* user_data_dir) {
  if (!PathService::Get(base::DIR_ANDROID_APP_DATA, user_data_dir)) {
    NOTREACHED() << "Failed to get app data directory for Android WebView";
  }
}

XWalkBrowserMainPartsAndroid::XWalkBrowserMainPartsAndroid(
    const content::MainFunctionParams& parameters)
    : XWalkBrowserMainParts(parameters) {
}

XWalkBrowserMainPartsAndroid::~XWalkBrowserMainPartsAndroid() {
}

void XWalkBrowserMainPartsAndroid::PreEarlyInitialization() {
  net::NetworkChangeNotifier::SetFactory(
      new net::NetworkChangeNotifierFactoryAndroid());
  // As Crosswalk uses in-process mode, that's easier than Chromium
  // to reach the default limit(1024) of open files per process on
  // Android. So increase the limit to 4096 explicitly.
  base::SetFdLimit(4096);

  // Initialize the Compositor.
  content::Compositor::Initialize();

  XWalkBrowserMainParts::PreEarlyInitialization();
}

void XWalkBrowserMainPartsAndroid::PreMainMessageLoopStart() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  // Disable ExtensionProcess for Android.
  // External extensions will run in the BrowserProcess (in process mode).
  command_line->AppendSwitch(switches::kXWalkDisableExtensionProcess);
  // Enable viewport.
  command_line->AppendSwitch(switches::kEnableViewport);
  // Temporary fix for XWALK-7231
  command_line->AppendSwitch(switches::kDisableUnifiedMediaPipeline);

  // Only force to enable WebGL for Android for IA platforms because
  // we've tested the WebGL conformance test. For other platforms, just
  // follow up the behavior defined by Chromium upstream.
#if defined(ARCH_CPU_X86) || defined(ARCH_CPU_X86_64)
  command_line->AppendSwitch(switches::kIgnoreGpuBlacklist);
#endif

#if defined(ENABLE_WEBRTC)
  // Disable HW encoding/decoding acceleration for WebRTC on Android.
  // FIXME: Remove these switches for Android when Android OS is removed from
  // GPU accelerated_video_decode blacklist or we stop ignoring the GPU
  // blacklist.
  command_line->AppendSwitch(switches::kDisableWebRtcHWDecoding);
  command_line->AppendSwitch(switches::kDisableWebRtcHWEncoding);
#endif

  // WebView does not (yet) save Chromium data during shutdown, so add setting
  // for Chrome to aggressively persist DOM Storage to minimize data loss.
  // http://crbug.com/479767
  command_line->AppendSwitch(switches::kEnableAggressiveDOMStorageFlushing);

  XWalkBrowserMainParts::PreMainMessageLoopStart();

  startup_url_ = GURL();
}

void XWalkBrowserMainPartsAndroid::PostMainMessageLoopStart() {
  base::MessageLoopForUI::current()->Start();
}

void XWalkBrowserMainPartsAndroid::PreMainMessageLoopRun() {
  net::NetModule::SetResourceProvider(PlatformResourceProvider);
  if (parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_default_message_loop_ = false;
  }

  xwalk_runner_->PreMainMessageLoopRun();

  extension_service_ = xwalk_runner_->extension_service();

  // Due to http://code.google.com/p/chromium/issues/detail?id=507809,
  // it's not possible to inject javascript into the main world by default.
  // So lift this limitation here to enable XWalkView.evaluateJavaScript
  // to work.
  content::RenderFrameHost::AllowInjectingJavaScriptForAndroidWebView();

  // Prepare the cookie store.
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kXWalkProfileName)) {
      base::FilePath user_data_dir;
      xwalk::GetUserDataDir(&user_data_dir);

      base::FilePath profile = user_data_dir.Append(
          command_line->GetSwitchValuePath(switches::kXWalkProfileName));
      MoveUserDataDirIfNecessary(user_data_dir, profile);
  }
}

void XWalkBrowserMainPartsAndroid::PostMainMessageLoopRun() {
  XWalkBrowserMainParts::PostMainMessageLoopRun();

  base::MessageLoopForUI::current()->Start();
}

void XWalkBrowserMainPartsAndroid::CreateInternalExtensionsForExtensionThread(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
  // On Android part, the ownership of each extension object will be transferred
  // to XWalkExtensionServer after this method is called. It is a rule enforced
  // by extension system that XWalkExtensionServer must own the extension
  // objects and extension instances.
  extensions::XWalkExtensionVector::const_iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it)
    extensions->push_back(*it);
}

void XWalkBrowserMainPartsAndroid::RegisterExtension(
    std::unique_ptr<XWalkExtension> extension) {
  // Since the creation of extension object is driven by Java side, and each
  // Java extension is backed by a native extension object. However, the Java
  // object may be destroyed by Android lifecycle management without destroying
  // the native side object. We keep the reference to native extension object
  // to make sure we can reuse the native object if Java extension is re-created
  // on resuming.
  extensions_.push_back(extension.release());
}

XWalkExtension* XWalkBrowserMainPartsAndroid::LookupExtension(
    const std::string& name) {
  extensions::XWalkExtensionVector::const_iterator it = extensions_.begin();
  for (; it != extensions_.end(); ++it) {
    XWalkExtension* extension = *it;
    if (name == extension->name()) return extension;
  }

  return NULL;
}

void XWalkBrowserMainPartsAndroid::RegisterExtensionInPath(
    const std::string& path) {
  extension_service_->RegisterExternalExtensionsForPath(
      base::FilePath(path));
}

}  // namespace xwalk
