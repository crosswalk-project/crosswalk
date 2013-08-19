// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "base/values.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScopedMicrotaskSuppression.h"
#include "v8/include/v8.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"

// This will be generated from xwalk_api.js.
extern const char kSource_xwalk_api[];

namespace xwalk {
namespace extensions {

XWalkExtensionRendererController::XWalkExtensionRendererController() {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->AddObserver(this);
  // TODO(cmarcelo): Once we have a better solution for the internal
  // extension helpers, remove this v8::Extension.
  thread->RegisterExtension(new v8::Extension("xwalk", kSource_xwalk_api));
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
  XWalkModuleSystem* module_system = new XWalkModuleSystem(context);
  XWalkModuleSystem::SetModuleSystemInContext(
      scoped_ptr<XWalkModuleSystem>(module_system), context);

  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForFrame(frame);
  handler->DidCreateScriptContext(frame);

  InstallJavaScriptAPIs(context, module_system);
}

void XWalkExtensionRendererController::WillReleaseScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context) {
  XWalkModuleSystem::ResetModuleSystemFromContext(context);
  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForFrame(frame);
  handler->WillReleaseScriptContext(frame);
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
    v8::Handle<v8::Context> context, XWalkModuleSystem* module_system) {
  v8::HandleScope handle_scope(context->GetIsolate());
  v8::Context::Scope context_scope(context);
  // FIXME(cmarcelo): Load extensions sorted by name so parent comes first, so
  // that we can safely register all them.
  ExtensionAPIMap::const_iterator it = extension_apis_.begin();
  for (; it != extension_apis_.end(); ++it) {
    if (it->second.empty())
      continue;
    scoped_ptr<XWalkExtensionModule> module(
        new XWalkExtensionModule(context, it->first, it->second));
    module_system->RegisterExtensionModule(context, module.Pass());
  }
}

}  // namespace extensions
}  // namespace xwalk
