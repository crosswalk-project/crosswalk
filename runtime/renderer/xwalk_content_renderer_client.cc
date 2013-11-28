// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_thread.h"
#include "grit/xwalk_sysapps_resources.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/renderer/application_native_module.h"
#include "xwalk/extensions/renderer/xwalk_js_module.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/renderer/android/xwalk_render_process_observer.h"
#include "xwalk/runtime/renderer/android/xwalk_render_view_ext.h"
#endif

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h"
#endif

namespace xwalk {

namespace {
XWalkContentRendererClient* g_renderer_client;
}

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

  WebKit::WebString application_scheme(
      ASCIIToUTF16(application::kApplicationScheme));
  WebKit::WebSecurityPolicy::registerURLSchemeAsSecure(application_scheme);
  WebKit::WebSecurityPolicy::registerURLSchemeAsCORSEnabled(application_scheme);

#if defined(OS_ANDROID)
  content::RenderThread* thread = content::RenderThread::Get();
  xwalk_render_process_observer_.reset(new XWalkRenderProcessObserver);
  thread->AddObserver(xwalk_render_process_observer_.get());
#endif
}

void XWalkContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
#if defined(OS_ANDROID)
  XWalkRenderViewExt::RenderViewCreated(render_view);
#endif
}

void XWalkContentRendererClient::DidCreateScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context,
    int extension_group, int world_id) {
  extension_controller_->DidCreateScriptContext(frame, context);
}

void XWalkContentRendererClient::WillReleaseScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context, int world_id) {
  extension_controller_->WillReleaseScriptContext(frame, context);
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
}

}  // namespace xwalk
