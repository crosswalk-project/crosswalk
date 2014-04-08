// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

#include "base/strings/utf_string_conversions.h"
#include "components/visitedlink/renderer/visitedlink_slave.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "content/public/renderer/render_thread.h"
#include "grit/xwalk_application_resources.h"
#include "grit/xwalk_sysapps_resources.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/renderer/application_native_module.h"
#include "xwalk/extensions/renderer/xwalk_js_module.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/renderer/android/xwalk_permission_client.h"
#include "xwalk/runtime/renderer/android/xwalk_render_process_observer.h"
#include "xwalk/runtime/renderer/android/xwalk_render_view_ext.h"
#else
#include "third_party/WebKit/public/web/WebFrame.h"
#endif

#if defined(OS_TIZEN)
#include "xwalk/runtime/common/xwalk_common_messages.h"
#endif

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h"
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
  virtual ~XWalkFrameHelper() {}

  // RenderFrameObserver implementation.
  virtual void WillReleaseScriptContext(v8::Handle<v8::Context> context,
                                        int world_id) OVERRIDE {
    extension_controller_->WillReleaseScriptContext(
        render_frame()->GetWebFrame(), context);
  }

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
}

void XWalkContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
#if defined(OS_ANDROID)
  XWalkRenderViewExt::RenderViewCreated(render_view);
#endif
}

void XWalkContentRendererClient::DidCreateScriptContext(
    blink::WebFrame* frame, v8::Handle<v8::Context> context,
    int extension_group, int world_id) {
  extension_controller_->DidCreateScriptContext(frame, context);
#if !defined(OS_ANDROID)
  xwalk_render_process_observer_->DidCreateScriptContext(
      frame, context, extension_group, world_id);
#endif
}

void XWalkContentRendererClient::DidCreateModuleSystem(
    extensions::XWalkModuleSystem* module_system) {
  scoped_ptr<extensions::XWalkNativeModule> app_module(
      new application::ApplicationNativeModule());
  module_system->RegisterNativeModule("application", app_module.Pass());
  module_system->RegisterNativeModule("sysapps_common",
      extensions::CreateJSModuleFromResource(IDR_XWALK_SYSAPPS_COMMON_API));
  module_system->RegisterNativeModule("sysapps_promise",
      extensions::CreateJSModuleFromResource(
          IDR_XWALK_SYSAPPS_COMMON_PROMISE_API));
  module_system->RegisterNativeModule("widget_common",
      extensions::CreateJSModuleFromResource(
          IDR_XWALK_APPLICATION_WIDGET_COMMON_API));
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
                     content::PageTransition transition_type,
                     const GURL& url,
                     const GURL& first_party_for_cookies,
                     GURL* new_url) {
#if defined(OS_ANDROID)
  return false;
#else
  if (!xwalk_render_process_observer_->IsWarpMode())
    return false;

  GURL origin_url(frame->document().url());
  GURL app_url(xwalk_render_process_observer_->app_url());
  if ((url.scheme() == app_url.scheme() &&
       url.host() == app_url.host()) ||
      frame->document().securityOrigin().canRequest(url)) {
    LOG(INFO) << "[PASS] " << origin_url.spec() << " request " << url.spec();
    return false;
  }

  LOG(INFO) << "[BLOCK] " << origin_url.spec() << " request " << url.spec();

#if defined(OS_TIZEN)
  if (origin_url.spec().empty())
    content::RenderThread::Get()->Send(new ViewMsg_OpenLinkExternal(url));
#endif

  *new_url = GURL();
  return true;
#endif
}
}  // namespace xwalk
