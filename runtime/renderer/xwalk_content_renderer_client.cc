// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "components/autofill/content/renderer/autofill_agent.h"
#include "components/autofill/content/renderer/password_autofill_agent.h"
#include "components/nacl/renderer/ppb_nacl_private_impl.h"
#include "components/visitedlink/renderer/visitedlink_slave.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "content/public/renderer/render_thread.h"
#include "grit/xwalk_application_resources.h"
#include "grit/xwalk_sysapps_resources.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/renderer/application_native_module.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/extensions/renderer/xwalk_js_module.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"
#include "xwalk/runtime/common/xwalk_localized_error.h"
#include "xwalk/runtime/renderer/isolated_file_system.h"
#include "xwalk/runtime/renderer/pepper/pepper_helper.h"

#if defined(OS_ANDROID)
#include "components/cdm/renderer/android_key_systems.h"
#include "xwalk/runtime/browser/android/net/url_constants.h"
#include "xwalk/runtime/renderer/android/xwalk_permission_client.h"
#include "xwalk/runtime/renderer/android/xwalk_render_process_observer.h"
#include "xwalk/runtime/renderer/android/xwalk_render_view_ext.h"
#else
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#endif

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h"
#endif

#if defined(OS_TIZEN)
#include "third_party/WebKit/public/web/WebScriptSource.h"
#include "xwalk/runtime/renderer/tizen/xwalk_render_view_ext_tizen.h"
#endif

#if !defined(DISABLE_NACL)
#include "components/nacl/renderer/nacl_helper.h"
#endif

namespace xwalk {

namespace {

xwalk::XWalkContentRendererClient* g_renderer_client;

class XWalkFrameHelper
    : public content::RenderFrameObserver,
      public content::RenderFrameObserverTracker<XWalkFrameHelper> {
 public:
  XWalkFrameHelper(
      content::RenderFrame* render_frame,
      extensions::XWalkExtensionRendererController* extension_controller)
      : content::RenderFrameObserver(render_frame),
        content::RenderFrameObserverTracker<XWalkFrameHelper>(render_frame),
        extension_controller_(extension_controller) {}
  ~XWalkFrameHelper() override {}

  // RenderFrameObserver implementation.
  void DidCreateScriptContext(v8::Handle<v8::Context> context,
                              int extension_group, int world_id) override {
    if (extension_controller_)
      extension_controller_->DidCreateScriptContext(
          render_frame()->GetWebFrame(), context);

#if defined(OS_TIZEN)
    const std::string code =
        "(function() {"
        "  window.eventListenerList = [];"
        "  window._addEventListener = window.addEventListener;"
        "  window.addEventListener = function(event, callback, useCapture) {"
        "    if (event == 'storage') {"
        "      window.eventListenerList.push(callback);"
        "    }"
        "    window._addEventListener(event, callback, useCapture);"
        "  }"
        "})();";
    const blink::WebScriptSource source =
      blink::WebScriptSource(base::ASCIIToUTF16(code));
    render_frame()->GetWebFrame()->executeScript(source);
#endif
  }
  void WillReleaseScriptContext(v8::Handle<v8::Context> context,
                                int world_id) override {
    if (extension_controller_)
      extension_controller_->WillReleaseScriptContext(
          render_frame()->GetWebFrame(), context);
  }

#if defined(OS_TIZEN)
  void DidCommitProvisionalLoad(bool is_new_navigation,
                                bool is_same_page_navigation) override {
    blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
    GURL url(frame->document().url());
    if (url.SchemeIs(application::kApplicationScheme)) {
      blink::WebSecurityOrigin origin = frame->document().securityOrigin();
      origin.grantLoadLocalResources();
    }
  }
#endif

 private:
  extensions::XWalkExtensionRendererController* extension_controller_;

  DISALLOW_COPY_AND_ASSIGN(XWalkFrameHelper);
};

}  // namespace

XWalkContentRendererClient* XWalkContentRendererClient::Get() {
  return g_renderer_client;
}

XWalkContentRendererClient::XWalkContentRendererClient() {
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

XWalkContentRendererClient::~XWalkContentRendererClient() {
  g_renderer_client = NULL;
}

void XWalkContentRendererClient::RenderThreadStarted() {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (!cmd_line->HasSwitch(switches::kXWalkDisableExtensions))
    extension_controller_.reset(
        new extensions::XWalkExtensionRendererController(this));

  blink::WebString application_scheme(
      base::ASCIIToUTF16(application::kApplicationScheme));
  blink::WebSecurityPolicy::registerURLSchemeAsSecure(application_scheme);
  blink::WebSecurityPolicy::registerURLSchemeAsCORSEnabled(application_scheme);

  content::RenderThread* thread = content::RenderThread::Get();
  xwalk_render_process_observer_.reset(new XWalkRenderProcessObserver);
  thread->AddObserver(xwalk_render_process_observer_.get());
#if defined(OS_ANDROID)
  blink::WebString content_scheme(
      base::ASCIIToUTF16(xwalk::kContentScheme));
  blink::WebSecurityPolicy::registerURLSchemeAsLocal(content_scheme);

  visited_link_slave_.reset(new visitedlink::VisitedLinkSlave);
  thread->AddObserver(visited_link_slave_.get());
#endif
}

void XWalkContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new XWalkFrameHelper(render_frame, extension_controller_.get());
#if defined(OS_ANDROID)
  new XWalkPermissionClient(render_frame);
#endif

#if defined(ENABLE_PLUGINS)
  new PepperHelper(render_frame);
#endif

#if !defined(DISABLE_NACL)
  new nacl::NaClHelper(render_frame);
#endif

  // The following code was copied from
  // android_webview/renderer/aw_content_renderer_client.cc
  // TODO(sgurun) do not create a password autofill agent (change
  // autofill agent to store a weakptr).
  autofill::PasswordAutofillAgent* password_autofill_agent =
      new autofill::PasswordAutofillAgent(render_frame);
  new autofill::AutofillAgent(render_frame, password_autofill_agent, nullptr);
}

void XWalkContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
#if defined(OS_ANDROID)
  XWalkRenderViewExt::RenderViewCreated(render_view);
#elif defined(OS_TIZEN)
  XWalkRenderViewExtTizen::RenderViewCreated(render_view);
#endif
}

void XWalkContentRendererClient::DidCreateModuleSystem(
    extensions::XWalkModuleSystem* module_system) {
  scoped_ptr<extensions::XWalkNativeModule> app_module(
      new application::ApplicationNativeModule());
  module_system->RegisterNativeModule("application", app_module.Pass());
  scoped_ptr<extensions::XWalkNativeModule> isolated_file_system_module(
      new extensions::IsolatedFileSystem());
  module_system->RegisterNativeModule("isolated_file_system",
      isolated_file_system_module.Pass());
  module_system->RegisterNativeModule("sysapps_common",
      extensions::CreateJSModuleFromResource(IDR_XWALK_SYSAPPS_COMMON_API));
  module_system->RegisterNativeModule("sysapps_promise",
      extensions::CreateJSModuleFromResource(
          IDR_XWALK_SYSAPPS_COMMON_PROMISE_API));
  module_system->RegisterNativeModule("widget_common",
      extensions::CreateJSModuleFromResource(
          IDR_XWALK_APPLICATION_WIDGET_COMMON_API));
}

const void* XWalkContentRendererClient::CreatePPAPIInterface(
    const std::string& interface_name) {
#if defined(ENABLE_PLUGINS) && !defined(DISABLE_NACL)
  if (interface_name == PPB_NACL_PRIVATE_INTERFACE)
    return nacl::GetNaClPrivateInterface();
#endif
  return NULL;
}

bool XWalkContentRendererClient::IsExternalPepperPlugin(
    const std::string& module_name) {
  // TODO(bbudge) remove this when the trusted NaCl plugin has been removed.
  // We must defer certain plugin events for NaCl instances since we switch
  // from the in-process to the out-of-process proxy after instantiating them.
  return module_name == "Native Client";
}

#if defined(OS_ANDROID)
unsigned long long XWalkContentRendererClient::VisitedLinkHash( // NOLINT
    const char* canonical_url, size_t length) {
  return visited_link_slave_->ComputeURLFingerprint(canonical_url, length);
}

bool XWalkContentRendererClient::IsLinkVisited(unsigned long long link_hash) { // NOLINT
  return visited_link_slave_->IsVisited(link_hash);
}
#endif

bool XWalkContentRendererClient::WillSendRequest(blink::WebFrame* frame,
                     ui::PageTransition transition_type,
                     const GURL& url,
                     const GURL& first_party_for_cookies,
                     GURL* new_url) {
#if defined(OS_ANDROID)
  return false;
#else
  if (!xwalk_render_process_observer_->IsWarpMode() &&
      !xwalk_render_process_observer_->IsCSPMode())
    return false;

  GURL origin_url(frame->document().url());
  GURL app_url(xwalk_render_process_observer_->app_url());
  // if under CSP mode.
  if (xwalk_render_process_observer_->IsCSPMode()) {
    if (!origin_url.is_empty() && origin_url != first_party_for_cookies &&
        !xwalk_render_process_observer_->CanRequest(app_url, url)) {
      LOG(INFO) << "[BLOCK] allow-navigation: " << url.spec();
      content::RenderThread::Get()->Send(new ViewMsg_OpenLinkExternal(url));
      *new_url = GURL();
      return true;
    }
    return false;
  }

  // if under WARP mode.
  if (url.GetOrigin() == app_url.GetOrigin() ||
      xwalk_render_process_observer_->CanRequest(app_url, url)) {
    DLOG(INFO) << "[PASS] " << origin_url.spec() << " request " << url.spec();
    return false;
  }

  LOG(INFO) << "[BLOCK] " << origin_url.spec() << " request " << url.spec();
#if defined(OS_TIZEN)
  if (url.GetOrigin() != app_url.GetOrigin() &&
      origin_url != first_party_for_cookies &&
      first_party_for_cookies.GetOrigin() != app_url.GetOrigin())
    content::RenderThread::Get()->Send(new ViewMsg_OpenLinkExternal(url));
#endif
  *new_url = GURL();
  return true;
#endif
}

void XWalkContentRendererClient::GetNavigationErrorStrings(
    content::RenderView* render_view,
    blink::WebFrame* frame,
    const blink::WebURLRequest& failed_request,
    const blink::WebURLError& error,
    std::string* error_html,
    base::string16* error_description) {
  bool is_post = base::EqualsASCII(failed_request.httpMethod(), "POST");

  // TODO(guangzhen): Check whether error_html is needed in xwalk runtime.

  if (error_description) {
    *error_description = LocalizedError::GetErrorDetails(error, is_post);
  }
}

void XWalkContentRendererClient::AddKeySystems(
    std::vector<media::KeySystemInfo>* key_systems) {
#if defined(OS_ANDROID)
  cdm::AddAndroidWidevine(key_systems);
#endif  // defined(OS_ANDROID)
}

}  // namespace xwalk
