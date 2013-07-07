// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScriptSource.h"
#include "v8/include/v8.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionV8Wrapper : public v8::Extension {
 public:
  XWalkExtensionV8Wrapper();

  // v8::Extension implementation.
  virtual v8::Handle<v8::FunctionTemplate> GetNativeFunction(
      v8::Handle<v8::String> name);

 private:
  static v8::Handle<v8::Value> PostMessage(const v8::Arguments& args);
};

// FIXME(cmarcelo): Implement this in C++ instead of JS. Tie it to the
// window object lifetime.
static const char* const kXWalkExtensionV8WrapperAPI =
    "var xwalk = xwalk || {};"
    "xwalk.postMessage = function(extension, msg) {"
    "  native function PostMessage();"
    "  PostMessage(extension, msg);"
    "};"
    "xwalk._message_listeners = {};"
    "xwalk.setMessageListener = function(extension, callback) {"
    "  if (callback === undefined)"
    "    delete xwalk._message_listeners[extension];"
    "  else"
    "    xwalk._message_listeners[extension] = callback;"
    "};"
    "xwalk.onpostmessage = function(extension, msg) {"
    "  var listener = xwalk._message_listeners[extension];"
    "  if (listener !== undefined)"
    "    listener(msg);"
    "};";

XWalkExtensionV8Wrapper::XWalkExtensionV8Wrapper()
    : v8::Extension("xwalk", kXWalkExtensionV8WrapperAPI) {
}

v8::Handle<v8::FunctionTemplate>
XWalkExtensionV8Wrapper::GetNativeFunction(v8::Handle<v8::String> name) {
  if (name->Equals(v8::String::New("PostMessage")))
    return v8::FunctionTemplate::New(PostMessage);
  return v8::Handle<v8::FunctionTemplate>();
}

v8::Handle<v8::Value> XWalkExtensionV8Wrapper::PostMessage(
    const v8::Arguments& args) {
  if (args.Length() != 2)
    return v8::False();

  std::string extension(*v8::String::Utf8Value(args[0]));
  std::string msg(*v8::String::Utf8Value(args[1]));

  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForCurrentContext();
  if (!handler->PostMessageToExtension(extension, msg))
    return v8::False();
  return v8::True();
}

XWalkExtensionRendererController::XWalkExtensionRendererController() {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->AddObserver(this);
  thread->RegisterExtension(new XWalkExtensionV8Wrapper);
}

XWalkExtensionRendererController::~XWalkExtensionRendererController() {
  content::RenderThread::Get()->RemoveObserver(this);
}

void XWalkExtensionRendererController::RenderViewCreated(
    content::RenderView* render_view) {
  // RenderView will own this object.
  new XWalkExtensionRenderViewHandler(render_view, this);
}

bool XWalkExtensionRendererController::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionRendererController, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_RegisterExtension, OnRegisterExtension)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionRendererController::OnRegisterExtension(
    const std::string& extension, const std::string& api) {
  extension_apis_[extension] = api;
}

bool XWalkExtensionRendererController::ContainsExtension(
    const std::string& extension) const {
  return extension_apis_.find(extension) != extension_apis_.end();
}

void XWalkExtensionRendererController::InstallJavaScriptAPIs(
    WebKit::WebFrame* frame) {
  ExtensionAPIMap::const_iterator it = extension_apis_.begin();
  for (; it != extension_apis_.end(); ++it) {
    const std::string& api_code = it->second;
    if (!api_code.empty())
      frame->executeScript(WebKit::WebScriptSource(
          WebKit::WebString::fromUTF8(api_code)));
  }
}

}  // namespace extensions
}  // namespace xwalk
