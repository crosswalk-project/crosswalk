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

v8::Handle<v8::Context> XWalkExtensionRenderViewHandler::GetContext() const {
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
  v8::Handle<v8::Context> context = GetContext();
  V8Context* v8_context = controller_->GetV8Context(context);
  XWalkExtensionMessageCore* message_core = controller_->message_core();
  message_core->OnPostMessage(v8_context, extension, msg);
}

}  // namespace extensions
}  // namespace xwalk
