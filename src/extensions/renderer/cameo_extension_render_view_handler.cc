// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/extensions/renderer/cameo_extension_render_view_handler.h"

#include "cameo/src/extensions/common/cameo_extension_messages.h"
#include "cameo/src/extensions/renderer/cameo_extension_renderer_controller.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "v8/include/v8.h"

namespace cameo {
namespace extensions {

CameoExtensionRenderViewHandler::CameoExtensionRenderViewHandler(
    content::RenderView* render_view,
    CameoExtensionRendererController* controller)
    : content::RenderViewObserver(render_view),
      content::RenderViewObserverTracker<CameoExtensionRenderViewHandler>(
          render_view),
      controller_(controller) {
  CHECK(controller);
}

CameoExtensionRenderViewHandler*
CameoExtensionRenderViewHandler::GetForCurrentContext() {
  WebKit::WebFrame* webframe = WebKit::WebFrame::frameForCurrentContext();
  if (!webframe) return NULL;

  WebKit::WebView* webview = webframe->view();
  if (!webview) return NULL;  // Can happen during closing.

  content::RenderView* render_view = content::RenderView::FromWebView(webview);
  return CameoExtensionRenderViewHandler::Get(render_view);
}

bool CameoExtensionRenderViewHandler::PostMessageToExtension(
    const std::string& extension, const std::string& msg) {
  return Send(new CameoViewHostMsg_PostMessage(routing_id(), extension, msg));
}

void CameoExtensionRenderViewHandler::DidClearWindowObject(
    WebKit::WebFrame* frame) {
  controller_->InstallJavaScriptAPIs(frame);
}

bool CameoExtensionRenderViewHandler::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CameoExtensionRenderViewHandler, message)
    IPC_MESSAGE_HANDLER(CameoViewMsg_PostMessage, OnPostMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void CameoExtensionRenderViewHandler::OnPostMessage(
    const std::string& extension, const std::string& msg) {
  if (!controller_->ContainsExtension(extension))
    return;

  WebKit::WebFrame* frame = render_view()->GetWebView()->mainFrame();

  v8::HandleScope handle_scope;
  const int argc = 2;
  v8::Handle<v8::Value> argv[argc] = {
    v8::String::New(extension.c_str()),
    v8::String::New(msg.c_str())
  };

  v8::Handle<v8::Context> context = frame->mainWorldScriptContext();
  v8::Context::Scope context_scope(context);

  // FIXME(cmarcelo): The way we are doing this, onpostmessage is exposed
  // and could be changed. An alternative design would be to expose those
  // things in a more controlled way during DidClearWindowObject instead of
  // using v8::Extension.
  v8::Handle<v8::Value> cameo =
      context->Global()->Get(v8::String::New("cameo"));
  v8::Handle<v8::Value> callback =
      cameo.As<v8::Object>()->Get(v8::String::New("onpostmessage"));

  // Note: see comment in WebScopedMicrotaskSuppression.h to understand why we
  // are not using V8 API directly but going through frame.
  frame->callFunctionEvenIfScriptDisabled(callback.As<v8::Function>(),
                                          context->Global(),
                                          argc, argv);
}

}  // namespace extensions
}  // namespace cameo
