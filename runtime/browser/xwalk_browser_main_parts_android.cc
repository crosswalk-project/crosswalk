// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"

#include "base/android/path_utils.h"
#include "base/base_paths_android.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/sys_info.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/sequenced_worker_pool.h"
#include "cc/base/switches.h"
#include "content/public/browser/android/compositor.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/result_codes.h"
#include "grit/net_resources.h"
#include "net/android/network_change_notifier_factory_android.h"
#include "net/base/network_change_notifier.h"
#include "net/base/net_module.h"
#include "net/base/net_util.h"
#include "net/cookies/cookie_store.h"
#include "ui/base/layout.h"
#include "ui/base/l10n/l10n_util_android.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/runtime/browser/android/cookie_manager.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_runtime_features.h"

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

using content::BrowserThread;
using extensions::XWalkExtension;

XWalkBrowserMainPartsAndroid::XWalkBrowserMainPartsAndroid(
    const content::MainFunctionParams& parameters)
    : XWalkBrowserMainParts(parameters) {
}

XWalkBrowserMainPartsAndroid::~XWalkBrowserMainPartsAndroid() {
}

void XWalkBrowserMainPartsAndroid::PreEarlyInitialization() {
  net::NetworkChangeNotifier::SetFactory(
      new net::NetworkChangeNotifierFactoryAndroid());

  CommandLine::ForCurrentProcess()->AppendSwitch(
      cc::switches::kCompositeToMailbox);

  // Initialize the Compositor.
  content::Compositor::Initialize();

  XWalkBrowserMainParts::PreEarlyInitialization();
}

void XWalkBrowserMainPartsAndroid::PreMainMessageLoopStart() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  // Disable ExtensionProcess for Android.
  // External extensions will run in the BrowserProcess (in process mode).
  command_line->AppendSwitch(switches::kXWalkDisableExtensionProcess);

  // Enable WebGL for Android.
  command_line->AppendSwitch(switches::kIgnoreGpuBlacklist);

  // Disable HW encoding/decoding acceleration for WebRTC on Android.
  // FIXME: Remove these switches for Android when Android OS is removed from
  // GPU accelerated_video_decode blacklist or we stop ignoring the GPU
  // blacklist.
  command_line->AppendSwitch(switches::kDisableWebRtcHWDecoding);
  command_line->AppendSwitch(switches::kDisableWebRtcHWEncoding);

  if (!command_line->HasSwitch(switches::kNumRasterThreads)) {
    // Enable multiple threads for rasterization to take advantage of multi CPU
    // cores. Only valid when ImplSidePainting is enabled.
    const int num_of_cpu_cores = base::SysInfo::NumberOfProcessors();
    command_line->AppendSwitchASCII(switches::kNumRasterThreads,
                                    base::IntToString(num_of_cpu_cores));
  }

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

  runtime_context_ = xwalk_runner_->runtime_context();
  extension_service_ = xwalk_runner_->extension_service();

  // Prepare the cookie store.
  base::FilePath user_data_dir;
  if (!PathService::Get(base::DIR_ANDROID_APP_DATA, &user_data_dir)) {
    NOTREACHED() << "Failed to get app data directory for Crosswalk";
  }

  base::FilePath cookie_store_path = user_data_dir.Append(
      FILE_PATH_LITERAL("Cookies"));
  scoped_refptr<base::SequencedTaskRunner> background_task_runner =
      BrowserThread::GetBlockingPool()->GetSequencedTaskRunner(
          BrowserThread::GetBlockingPool()->GetSequenceToken());

  content::CookieStoreConfig cookie_config(
      cookie_store_path,
      content::CookieStoreConfig::RESTORED_SESSION_COOKIES,
      NULL, NULL);
  cookie_config.client_task_runner = BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO);
  cookie_config.background_task_runner = background_task_runner;
  net::CookieStore* cookie_store = content::CreateCookieStore(cookie_config);
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
    scoped_ptr<XWalkExtension> extension) {
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

}  // namespace xwalk
