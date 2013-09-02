// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "base/values.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/v8_value_converter.h"
#include "ipc/ipc_sync_channel.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "v8/include/v8.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_client.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_extension_render_view_handler.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"
#include "xwalk/extensions/renderer/xwalk_remote_extension_runner.h"
#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

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

  in_browser_process_extensions_client_.reset(new XWalkExtensionClient(
      thread->GetChannel()));
}

XWalkExtensionRendererController::~XWalkExtensionRendererController() {
  // FIXME(cmarcelo): These call is causing crashes on shutdown with Chromium
  //                  29.0.1547.57 and had to be commented out.
  // content::RenderThread::Get()->RemoveObserver(this);
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

  module_system->RegisterNativeModule(
      "v8tools", scoped_ptr<XWalkNativeModule>(new XWalkV8ToolsModule));

  v8::HandleScope handle_scope(context->GetIsolate());
  v8::Context::Scope context_scope(context);
  // FIXME(cmarcelo): Load extensions sorted by name so parent comes first, so
  // that we can safely register all them.
  ExtensionAPIMap::const_iterator it = extension_apis_.begin();
  for (; it != extension_apis_.end(); ++it) {
    if (it->second.empty())
      continue;
    scoped_ptr<XWalkExtensionModule> module(
        new XWalkExtensionModule(module_system, it->first, it->second));
    XWalkRemoteExtensionRunner* runner =
        in_browser_process_extensions_client_->CreateRunner(
        handler, frame->identifier(), it->first, module.get());
    module->set_runner(runner);
    runners_.push_back(runner); // FIXME(jeez): remove this.
    module_system->RegisterExtensionModule(module.Pass());
  }
}

void XWalkExtensionRendererController::WillReleaseScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context) {
  XWalkModuleSystem::ResetModuleSystemFromContext(context);
  XWalkExtensionRenderViewHandler* handler =
      XWalkExtensionRenderViewHandler::GetForFrame(frame);
  handler->WillReleaseScriptContext(frame);

  const int64_t frame_id = frame->identifier();
  RunnerVector::iterator it = runners_.begin();
  while (it != runners_.end()) {
    XWalkRemoteExtensionRunner* runner = *it;
    if (runner->frame_id() == frame_id) {
      delete runner;
      runners_.erase(it);
    } else {
      ++it;
    }
  }
}

bool XWalkExtensionRendererController::OnControlMessageReceived(
    const IPC::Message& message) {
  // FIXME(jeez): pass the RegisterExtension Messages to in_browser_process_extensions_client_.
  if (in_browser_process_extensions_client_->OnMessageReceived(message))
    return true;

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionRendererController, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_RegisterExtension, OnRegisterExtension)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

XWalkRemoteExtensionRunner* XWalkExtensionRendererController::GetRunner(
    XWalkExtensionRenderViewHandler* handler, int64_t frame_id,
    const std::string& extension_name) {
  XWalkRemoteExtensionRunner* result = NULL;
  RunnerVector::iterator it = runners_.begin();
  for (; it != runners_.end(); ++it) {
    XWalkRemoteExtensionRunner* runner = *it;
    if (runner->handler() == handler && runner->frame_id() == frame_id
        && runner->extension_name() == extension_name) {
      result = runner;
      break;
    }
  }
  CHECK(result);
  return result;
}

void XWalkExtensionRendererController::OnRegisterExtension(
    const std::string& extension, const std::string& api) {
  extension_apis_[extension] = api;

  // FIXME(jeez): we will pass the OnRegisterExtension directly to this client,
  // so this has to be removed.
  in_browser_process_extensions_client_->OnRegisterExtension(extension, api);
}

bool XWalkExtensionRendererController::ContainsExtension(
    const std::string& extension) const {
  return extension_apis_.find(extension) != extension_apis_.end();
}

}  // namespace extensions
}  // namespace xwalk
