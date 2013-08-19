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

XWalkExtensionRenderViewHandler::~XWalkExtensionRenderViewHandler() {
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

namespace {

// Regular base::Value doesn't have param traits, so can't be passed as is
// through IPC. We wrap it in a base::ListValue that have traits before
// exchanging.
//
// Implementing param traits for base::Value is not a viable option at the
// moment (would require fork base::Value and create a new empty type).
scoped_ptr<base::ListValue> WrapValueInList(scoped_ptr<base::Value> value) {
  if (!value)
    return scoped_ptr<base::ListValue>();
  scoped_ptr<base::ListValue> list_value(new base::ListValue);
  list_value->Append(value.release());
  return list_value.Pass();
}

}  // namespace

bool XWalkExtensionRenderViewHandler::PostMessageToExtension(
    int64_t frame_id, const std::string& extension,
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::ListValue> wrapped_msg = WrapValueInList(msg.Pass());
  return Send(
      new XWalkViewHostMsg_PostMessage(routing_id(), frame_id,
                                       extension, *wrapped_msg));
}

scoped_ptr<base::Value>
XWalkExtensionRenderViewHandler::SendSyncMessageToExtension(
    int64_t frame_id, const std::string& extension,
    scoped_ptr<base::Value> msg) {
  scoped_ptr<base::ListValue> wrapped_msg = WrapValueInList(msg.Pass());
  base::ListValue* wrapped_reply = new base::ListValue;
  Send(new XWalkViewHostMsg_SendSyncMessage(
      routing_id(), frame_id, extension, *wrapped_msg, wrapped_reply));

  base::Value* reply;
  wrapped_reply->Remove(0, &reply);
  return scoped_ptr<base::Value>(reply);
}

void XWalkExtensionRenderViewHandler::DidCreateScriptContext(
    WebKit::WebFrame* frame) {
  int64_t frame_id = frame->identifier();
  CHECK(id_to_frame_map_.find(frame_id) == id_to_frame_map_.end());
  id_to_frame_map_[frame_id] = frame;
  Send(new XWalkViewHostMsg_DidCreateScriptContext(routing_id(), frame_id));
}

void XWalkExtensionRenderViewHandler::WillReleaseScriptContext(
    WebKit::WebFrame* frame) {
  int64_t frame_id = frame->identifier();
  CHECK(id_to_frame_map_.find(frame_id) != id_to_frame_map_.end());
  id_to_frame_map_.erase(frame_id);
  Send(new XWalkViewHostMsg_WillReleaseScriptContext(routing_id(), frame_id));
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

void XWalkExtensionRenderViewHandler::OnPostMessage(int64_t frame_id,
    const std::string& extension, const base::ListValue& msg) {
  if (!controller_->ContainsExtension(extension))
    return;

  v8::HandleScope handle_scope;
  v8::Handle<v8::Context> context = GetV8ContextForFrame(frame_id);
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

v8::Handle<v8::Context> XWalkExtensionRenderViewHandler::GetV8ContextForFrame(
    int64_t frame_id) {
  IdToFrameMap::iterator it = id_to_frame_map_.find(frame_id);
  CHECK(it != id_to_frame_map_.end());
  return it->second->mainWorldScriptContext();
}

}  // namespace extensions
}  // namespace xwalk
