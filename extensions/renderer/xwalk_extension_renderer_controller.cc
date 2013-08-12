// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "base/stringprintf.h"
#include "base/values.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScriptSource.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScopedMicrotaskSuppression.h"
#include "v8/include/v8.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_message_core.h"
#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"
#include "xwalk/extensions/renderer/xwalk_extension_v8_context.h"

namespace xwalk {
namespace extensions {

XWalkExtensionRendererController::XWalkExtensionRendererController() {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->AddObserver(this);
  message_core_.reset(new XWalkExtensionMessageCore());
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
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context) {
  V8Context* v8_context = new V8Context(context, frame);
  v8_context_set_.Add(v8_context);
  InstallJavaScriptAPIs(frame);
}

void XWalkExtensionRendererController::WillReleaseScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context) {
  V8Context* v8_context = v8_context_set_.Get(context);
  DCHECK(v8_context);

  message_core_->OnWillReleaseScriptContext(v8_context);
  v8_context_set_.Remove(v8_context);
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

V8Context* XWalkExtensionRendererController::GetV8Context(
    v8::Handle<v8::Context> context) const {
  return v8_context_set_.Get(context);
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
    result += ns + " = " + "window." + ns + " || {}; ";
    pos++;
  }
  return result;
}

void XWalkExtensionRendererController::WrapAndInjectAPICode(
    WebKit::WebFrame* frame,
    const std::string& api_code,
    const std::string& extension_name) {
  // We take care here to make sure that line numbering for api_code after
  // wrapping doesn't change, so that syntax errors point to the correct line.
  const char* name = extension_name.c_str();

  // Note that we are using the Post to indicate an asynchronous call and the
  // term Send to indicate synchronous call.
  //
  // FIXME(cmarcelo): For now it is disabled on Windows because we jump through
  // the UI process and this is not supported in that platform. See issue
  // https://github.com/otcshare/crosswalk/issues/268 for details.
  std::string api_wrapper = base::StringPrintf(
    "(function() {"
    "  return function (extension) {"
    "    %s"
    "    var exports = %s;"
    "    'use strict'; %s\n};"
    "})()",
    CodeToEnsureNamespace(extension_name).c_str(),
    name,
    api_code.c_str()
  ); // NOLINT

  v8::HandleScope handle_scope;
  v8::Handle<v8::Context> context = frame->mainWorldScriptContext();

  V8Context* v8_context = v8_context_set_.Get(context);
  DCHECK(v8_context);
  v8::Context::Scope context_scope(context);
  v8::Handle<v8::Object> global = context->Global();

  // prepare message core wrapper object
  v8::Handle<v8::Value> message_core =
    message_core_->GetMessageCoreWrapper(v8_context, extension_name);

  WebKit::WebScopedMicrotaskSuppression suppression;
  // pass wrapper object as the api wrapper's argument
  v8::Handle<v8::Value> api_func = v8::Script::Compile(
      v8::String::New(api_wrapper.c_str()))->Run();
  v8::Handle<v8::Value> api_func_args[] = { message_core.As<v8::Object>() };
  frame->callFunctionEvenIfScriptDisabled(
      api_func.As<v8::Function>(), global, 1, api_func_args);
}

void XWalkExtensionRendererController::InstallJavaScriptAPIs(
    WebKit::WebFrame* frame) {
  // FIXME(cmarcelo): Load extensions sorted by name so parent comes first, so
  // that we can safely register all them.
  ExtensionAPIMap::const_iterator it = extension_apis_.begin();
  for (; it != extension_apis_.end(); ++it) {
    const std::string& extension_name = it->first;
    const std::string& api_code = it->second;
    if (!api_code.empty())
      WrapAndInjectAPICode(frame, api_code, extension_name);
  }
}

}  // namespace extensions
}  // namespace xwalk
