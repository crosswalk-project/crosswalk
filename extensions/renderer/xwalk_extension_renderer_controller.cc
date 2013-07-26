// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "base/stringprintf.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScriptSource.h"
#include "v8/include/v8.h"

// This will be generated from xwalk_api.js.
extern const char kSource_xwalk_api[];

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
  static v8::Handle<v8::Value> SendSyncMessage(const v8::Arguments& args);
};

XWalkExtensionV8Wrapper::XWalkExtensionV8Wrapper()
    : v8::Extension("xwalk", kSource_xwalk_api) {
}

v8::Handle<v8::FunctionTemplate>
XWalkExtensionV8Wrapper::GetNativeFunction(v8::Handle<v8::String> name) {
  if (name->Equals(v8::String::New("PostMessage")))
    return v8::FunctionTemplate::New(PostMessage);
  if (name->Equals(v8::String::New("SendSyncMessage")))
    return v8::FunctionTemplate::New(SendSyncMessage);
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

v8::Handle<v8::Value> XWalkExtensionV8Wrapper::SendSyncMessage(
    const v8::Arguments& args) {
  if (args.Length() != 2)
    return v8::False();

  std::string extension(*v8::String::Utf8Value(args[0]));
  std::string msg(*v8::String::Utf8Value(args[1]));

  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForCurrentContext();
  std::string reply = handler->SendSyncMessageToExtension(extension, msg);
  return v8::String::New(reply.c_str(), reply.size());
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

void XWalkExtensionRendererController::DidCreateScriptContext(
    WebKit::WebFrame* frame) {
  InstallJavaScriptAPIs(frame);
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

static std::string CodeToEnsureNamespace(
    const std::string& extension_name) {
  std::string result;
  size_t pos = 0;
  while (true) {
    pos = extension_name.find('.', pos);
    if (pos == std::string::npos) {
      result += extension_name + " = {};";
      break;
    }
    std::string ns = extension_name.substr(0, pos);
    result += ns + " = " + ns + " || {}; ";
    pos++;
  }
  return result;
}

static std::string WrapAPICode(const std::string& api_code,
                               const std::string& extension_name) {
  // FIXME(cmarcelo): New extension.postMessage and extension.setMessageListener
  // should be implemented in a way that we don't need to expose
  // xwalk.postMessage and xwalk.setMessageListener.

  // We take care here to make sure that line numbering for api_code after
  // wrapping doesn't change, so that syntax errors point to the correct line.
  const char* name = extension_name.c_str();

  // Note that we are using the Post to indicate an asynchronous call and the
  // term Send to indicate synchronous call.
  //
  // FIXME(cmarcelo): For now it is disabled on Windows because we jump through
  // the UI process and this is not supported in that platform. See issue
  // https://github.com/otcshare/crosswalk/issues/268 for details.
  return base::StringPrintf(
      "var %s; (function(exports, extension) {'use strict'; %s\n})"
      "(%s, "
      "{ postMessage: function(msg) { xwalk.postMessage('%s', msg); },"
      "  setMessageListener: function(listener) { "
      "                        xwalk.setMessageListener('%s', listener); }"
#if !defined(OS_WIN)
      "  , internal: {"
      "    sendSyncMessage: function(msg) {"
      "      return xwalk.sendSyncMessage('%s', msg); }"
      "  }"
#endif
      "});",
      CodeToEnsureNamespace(extension_name).c_str(),
      api_code.c_str(),
      name, name, name
#if !defined(OS_WIN)
      , name
#endif
      );
}

void XWalkExtensionRendererController::InstallJavaScriptAPIs(
    WebKit::WebFrame* frame) {
  // FIXME(cmarcelo): Load extensions sorted by name so parent comes first, so
  // that we can safely register all them.
  ExtensionAPIMap::const_iterator it = extension_apis_.begin();
  for (; it != extension_apis_.end(); ++it) {
    const std::string& extension_name = it->first;
    const std::string& api_code = it->second;
    if (!api_code.empty()) {
      std::string wrapped_api_code = WrapAPICode(api_code, extension_name);
      frame->executeScript(WebKit::WebScriptSource(
          WebKit::WebString::fromUTF8(wrapped_api_code),
          WebKit::WebURL("JS API code for " + extension_name,
                         url_parse::Parsed(), false)));
    }
  }
}

}  // namespace extensions
}  // namespace xwalk
