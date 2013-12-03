// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/renderer/xwalk_extension_renderer_controller.h"

#include "base/command_line.h"
#include "base/values.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/v8_value_converter.h"
#include "grit/xwalk_extensions_resources.h"
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
#include "xwalk/extensions/renderer/xwalk_js_module.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"
#include "xwalk/extensions/renderer/xwalk_v8tools_module.h"

namespace xwalk {
namespace extensions {

const GURL kAboutBlankURL = GURL("about:blank");

XWalkExtensionRendererController::XWalkExtensionRendererController(
    Delegate* delegate)
    : shutdown_event_(false, false),
      delegate_(delegate) {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->AddObserver(this);

  IPC::SyncChannel* browser_channel = thread->GetChannel();
  SetupBrowserProcessClient(browser_channel);

  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkDisableExtensionProcess))
    LOG(INFO) << "EXTENSION PROCESS DISABLED.";
  else
    SetupExtensionProcessClient(browser_channel);
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
    XWalkExtensionClient::ExtensionCodePoints* codepoint = it->second;
    if (codepoint->api.empty())
      continue;
    scoped_ptr<XWalkExtensionModule> module(
        new XWalkExtensionModule(client, module_system,
                                 it->first, codepoint->api));
    module_system->RegisterExtensionModule(module.Pass(),
                                           codepoint->entry_points);
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
  module_system->RegisterNativeModule(
      "internal", CreateJSModuleFromResource(
          IDR_XWALK_EXTENSIONS_INTERNAL_API));

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
  return in_browser_process_extensions_client_->OnMessageReceived(message);
}

void XWalkExtensionRendererController::OnRenderProcessShutdown() {
  shutdown_event_.Signal();
}

void XWalkExtensionRendererController::SetupBrowserProcessClient(
    IPC::SyncChannel* browser_channel) {
  in_browser_process_extensions_client_.reset(new XWalkExtensionClient);
  in_browser_process_extensions_client_->Initialize(browser_channel);
}

void XWalkExtensionRendererController::SetupExtensionProcessClient(
    IPC::SyncChannel* browser_channel) {
  IPC::ChannelHandle handle;
  browser_channel->Send(
      new XWalkExtensionProcessHostMsg_GetExtensionProcessChannel(&handle));
  // FIXME(cmarcelo): Need to account for failure in creating the channel.

  external_extensions_client_.reset(new XWalkExtensionClient);
  extension_process_channel_.reset(new IPC::SyncChannel(handle,
      IPC::Channel::MODE_CLIENT, external_extensions_client_.get(),
      content::RenderThread::Get()->GetIOMessageLoopProxy(), true,
      &shutdown_event_));

  external_extensions_client_->Initialize(extension_process_channel_.get());
}


}  // namespace extensions
}  // namespace xwalk
