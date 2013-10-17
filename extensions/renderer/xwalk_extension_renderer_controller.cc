// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "base/command_line.h"
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
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/extensions/renderer/xwalk_extension_client.h"
#include "xwalk/extensions/renderer/xwalk_extension_module.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"
#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

// This will be generated from xwalk_api.js.
extern const char kSource_xwalk_api[];

namespace xwalk {
namespace extensions {

const GURL kAboutBlankURL = GURL("about:blank");

XWalkExtensionRendererController::XWalkExtensionRendererController(
    Delegate* delegate)
    : shutdown_event_(false, false),
      delegate_(delegate) {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->AddObserver(this);
  // TODO(cmarcelo): Once we have a better solution for the internal
  // extension helpers, remove this v8::Extension.
  thread->RegisterExtension(new v8::Extension("xwalk", kSource_xwalk_api));

  in_browser_process_extensions_client_.reset(new XWalkExtensionClient());
  in_browser_process_extensions_client_->Initialize(thread->GetChannel());

  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkEnableLoadingExtensionsOnDemand)) {
    LOG(INFO) << "LOADING EXTENSIONS ON DEMAND.";
  }
}

XWalkExtensionRendererController::~XWalkExtensionRendererController() {
  // FIXME(cmarcelo): These call is causing crashes on shutdown with Chromium
  //                  29.0.1547.57 and had to be commented out.
  // content::RenderThread::Get()->RemoveObserver(this);
}

namespace {

void CreateExtensionModules(XWalkExtensionClient* client,
                            XWalkModuleSystem* module_system) {
  const XWalkExtensionClient::ExtensionAPIMap& extensions =
      client->extension_apis();
  XWalkExtensionClient::ExtensionAPIMap::const_iterator it = extensions.begin();
  for (; it != extensions.end(); ++it) {
    if (it->second.empty())
      continue;
    scoped_ptr<XWalkExtensionModule> module(
        new XWalkExtensionModule(client, module_system, it->first, it->second));
    module_system->RegisterExtensionModule(module.Pass());
  }
}

}  // namespace

void XWalkExtensionRendererController::DidCreateScriptContext(
    WebKit::WebFrame* frame, v8::Handle<v8::Context> context) {
  XWalkModuleSystem* module_system = new XWalkModuleSystem(context);
  XWalkModuleSystem::SetModuleSystemInContext(
      scoped_ptr<XWalkModuleSystem>(module_system), context);

  module_system->RegisterNativeModule(
      "v8tools", scoped_ptr<XWalkNativeModule>(new XWalkV8ToolsModule));

  delegate_->DidCreateModuleSystem(module_system);

  CreateExtensionModules(in_browser_process_extensions_client_.get(),
                         module_system);

  if (external_extensions_client_) {
    CreateExtensionModules(external_extensions_client_.get(),
                           module_system);
  }

  module_system->Initialize();
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
