// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"

#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"

namespace xwalk {
namespace extensions {

XWalkExtensionRenderViewHandler::XWalkExtensionRenderViewHandler(
    content::RenderView* render_view,
    XWalkExtensionRendererController* controller)
    : content::RenderViewObserver(render_view),
      content::RenderViewObserverTracker<XWalkExtensionRenderViewHandler>(
          render_view),
      controller_(controller) {
  CHECK(controller);
}

XWalkExtensionRenderViewHandler*
XWalkExtensionRenderViewHandler::GetForCurrentContext() {
  WebKit::WebFrame* webframe = WebKit::WebFrame::frameForCurrentContext();
  if (!webframe) return NULL;

  WebKit::WebView* webview = webframe->view();
  if (!webview) return NULL;  // Can happen during closing.

  content::RenderView* render_view = content::RenderView::FromWebView(webview);
  return XWalkExtensionRenderViewHandler::Get(render_view);
}

XWalkExtensionRenderViewHandler*
XWalkExtensionRenderViewHandler::GetForFrame(WebKit::WebFrame* webframe) {
  WebKit::WebView* webview = webframe->view();
  if (!webview) return NULL;
  content::RenderView* render_view = content::RenderView::FromWebView(webview);
  return XWalkExtensionRenderViewHandler::Get(render_view);
}

v8::Handle<v8::Context> XWalkExtensionRenderViewHandler::GetV8Context() const {
  WebKit::WebFrame* frame = render_view()->GetWebView()->mainFrame();
  return frame->mainWorldScriptContext();
}

bool XWalkExtensionRenderViewHandler::PostMessageToExtension(
    const std::string& extension, const base::ListValue& msg) {
  return Send(new XWalkViewHostMsg_PostMessage(routing_id(), extension, msg));
}

scoped_ptr<base::ListValue>
XWalkExtensionRenderViewHandler::SendSyncMessageToExtension(
    const std::string& extension, const base::ListValue& msg) {
  base::ListValue* reply = new base::ListValue;
  Send(new XWalkViewHostMsg_SendSyncMessage(
      routing_id(), extension, msg, reply));
  return scoped_ptr<base::ListValue>(reply);
}

void XWalkExtensionRenderViewHandler::DidCreateScriptContext() {
  Send(new XWalkViewHostMsg_DidCreateScriptContext(routing_id()));
}

void XWalkExtensionRenderViewHandler::WillReleaseScriptContext() {
  Send(new XWalkViewHostMsg_WillReleaseScriptContext(routing_id()));
}

bool XWalkExtensionRenderViewHandler::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionRenderViewHandler, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_PostMessage, OnPostMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionRenderViewHandler::OnPostMessage(
    const std::string& extension, const base::ListValue& msg) {
  if (!controller_->ContainsExtension(extension))
    return;

  v8::HandleScope handle_scope;
  v8::Handle<v8::Context> context = GetV8Context();
  v8::Context::Scope context_scope(context);

  // We get the message wrapped in a ListValue because Value doesn't have
  // param traits.
  const base::Value* value;
  msg.Get(0, &value);

  scoped_ptr<content::V8ValueConverter> converter(
      content::V8ValueConverter::create());
  v8::Handle<v8::Value> v8_value(
      converter->ToV8Value(value, context));

  const int argc = 2;
  v8::Handle<v8::Value> argv[argc] = {
    v8::String::New(extension.c_str()),
    v8_value
  };

  // FIXME(cmarcelo): The way we are doing this, onpostmessage is exposed
  // and could be changed. An alternative design would be to expose those
  // things in a more controlled way during DidClearWindowObject instead of
  // using v8::Extension.
  v8::Handle<v8::Value> xwalk =
      context->Global()->Get(v8::String::New("xwalk"));
  v8::Handle<v8::Value> callback =
      xwalk.As<v8::Object>()->Get(v8::String::New("onpostmessage"));

  // Note: see comment in WebScopedMicrotaskSuppression.h to understand why we
  // are not using V8 API directly but going through frame.
  WebKit::WebFrame* frame = render_view()->GetWebView()->mainFrame();
  frame->callFunctionEvenIfScriptDisabled(callback.As<v8::Function>(),
                                          context->Global(),
                                          argc, argv);
}

}  // namespace extensions
}  // namespace xwalk
