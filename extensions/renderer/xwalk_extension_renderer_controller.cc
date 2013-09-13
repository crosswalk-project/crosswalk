// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "base/values.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/v8_value_converter.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_sync_channel.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "v8/include/v8.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/renderer/xwalk_extension_client.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"
#include "xwalk/extensions/renderer/xwalk_remote_extension_runner.h"
#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

// This will be generated from xwalk_api.js.
extern const char kSource_xwalk_api[];

namespace xwalk {
namespace extensions {

const GURL kAboutBlankURL = GURL("about:blank");

XWalkExtensionRendererController::XWalkExtensionRendererController()
    : shutdown_event_(false, false) {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->AddObserver(this);
  // TODO(cmarcelo): Once we have a better solution for the internal
  // extension helpers, remove this v8::Extension.
  thread->RegisterExtension(new v8::Extension("xwalk", kSource_xwalk_api));

  in_browser_process_extensions_client_.reset(new XWalkExtensionClient());
  in_browser_process_extensions_client_->Initialize(thread->GetChannel());
}

XWalkExtensionRendererController::~XWalkExtensionRendererController() {
  // FIXME(cmarcelo): These call is causing crashes on shutdown with Chromium
  //                  29.0.1547.57 and had to be commented out.
  // content::RenderThread::Get()->RemoveObserver(this);
}

void XWalkExtensionRendererController::DidCreateScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context) {
  XWalkModuleSystem* module_system = new XWalkModuleSystem(context);
  XWalkModuleSystem::SetModuleSystemInContext(
      scoped_ptr<XWalkModuleSystem>(module_system), context);

  module_system->RegisterNativeModule(
      "v8tools", scoped_ptr<XWalkNativeModule>(new XWalkV8ToolsModule));

  in_browser_process_extensions_client_->CreateRunnersForModuleSystem(
      module_system);

  if (external_extensions_client_)
    external_extensions_client_->CreateRunnersForModuleSystem(module_system);
}

void XWalkExtensionRendererController::WillReleaseScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context) {
  XWalkModuleSystem::ResetModuleSystemFromContext(context);
}

bool XWalkExtensionRendererController::OnControlMessageReceived(
    const IPC::Message& message) {
  if (in_browser_process_extensions_client_->OnMessageReceived(message))
    return true;

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(XWalkExtensionRendererController, message)
    IPC_MESSAGE_HANDLER(XWalkViewMsg_ExtensionProcessChannelCreated,
        OnExtensionProcessChannelCreated)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void XWalkExtensionRendererController::OnExtensionProcessChannelCreated(
    const IPC::ChannelHandle& handle) {
  external_extensions_client_.reset(new XWalkExtensionClient());

  extension_process_channel_.reset(new IPC::SyncChannel(handle,
      IPC::Channel::MODE_CLIENT, external_extensions_client_.get(),
      content::RenderThread::Get()->GetIOMessageLoopProxy(), true,
      &shutdown_event_));

  external_extensions_client_->Initialize(extension_process_channel_.get());
}

void XWalkExtensionRendererController::OnRenderProcessShutdown() {
  shutdown_event_.Signal();
}

}  // namespace extensions
}  // namespace xwalk
