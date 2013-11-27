// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/extension/application_runtime_extension.h"

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "ipc/ipc_message.h"
#include "grit/xwalk_application_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application.h"
#include "xwalk/runtime/browser/runtime.h"

using content::BrowserThread;

namespace xwalk {

ApplicationRuntimeExtension::ApplicationRuntimeExtension(
    application::ApplicationSystem* application_system)
  : application_system_(application_system) {
  set_name("xwalk.app.runtime");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_APPLICATION_RUNTIME_API).as_string());
}

XWalkExtensionInstance* ApplicationRuntimeExtension::CreateInstance() {
  return new AppRuntimeExtensionInstance(application_system_);
}

AppRuntimeExtensionInstance::AppRuntimeExtensionInstance(
    application::ApplicationSystem* application_system)
  : application_system_(application_system),
    handler_(this) {
  handler_.Register(
      "getManifest",
      base::Bind(&AppRuntimeExtensionInstance::OnGetManifest,
                 base::Unretained(this)));
  handler_.Register(
      "getMainDocumentID",
      base::Bind(&AppRuntimeExtensionInstance::OnGetMainDocumentID,
                 base::Unretained(this)));
}

void AppRuntimeExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void AppRuntimeExtensionInstance::OnGetManifest(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  base::DictionaryValue** manifest_data = new DictionaryValue*(NULL);
  BrowserThread::PostTaskAndReply(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(&AppRuntimeExtensionInstance::GetManifest,
                 base::Unretained(this),
                 base::Unretained(manifest_data)),
      base::Bind(&AppRuntimeExtensionInstance::PostManifest,
                 base::Unretained(this),
                 base::Passed(&info),
                 base::Owned(manifest_data)));
}

void AppRuntimeExtensionInstance::OnGetMainDocumentID(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  int* main_routing_id = new int(MSG_ROUTING_NONE);
  BrowserThread::PostTaskAndReply(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(&AppRuntimeExtensionInstance::GetMainDocumentID,
                 base::Unretained(this),
                 base::Unretained(main_routing_id)),
      base::Bind(&AppRuntimeExtensionInstance::PostMainDocumentID,
                 base::Unretained(this),
                 base::Passed(&info),
                 base::Owned(main_routing_id)));
}

void AppRuntimeExtensionInstance::GetManifest(
    base::DictionaryValue** manifest_data) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const application::ApplicationService* service =
    application_system_->application_service();
  const application::Application* app = service->GetRunningApplication();
  if (app)
    *manifest_data = app->GetManifest()->value()->DeepCopy();
}

void AppRuntimeExtensionInstance::PostManifest(
    scoped_ptr<XWalkExtensionFunctionInfo> info,
    base::DictionaryValue** manifest_data) {
  DCHECK(thread_checker_.CalledOnValidThread());
  scoped_ptr<base::ListValue> results(new base::ListValue());
  if (*manifest_data)
    results->Append(*manifest_data);
  else
    // Return an empty dictionary value when there's no valid manifest data.
    results->Append(new base::DictionaryValue());
  info->PostResult(results.Pass());
}

void AppRuntimeExtensionInstance::GetMainDocumentID(int* main_routing_id) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const application::ApplicationProcessManager* pm =
    application_system_->process_manager();
  const Runtime* runtime = pm->GetMainDocumentRuntime();
  if (runtime)
    *main_routing_id = runtime->web_contents()->GetRoutingID();
}

void AppRuntimeExtensionInstance::PostMainDocumentID(
    scoped_ptr<XWalkExtensionFunctionInfo> info, int* main_routing_id) {
  DCHECK(thread_checker_.CalledOnValidThread());
  scoped_ptr<base::ListValue> results(new base::ListValue());
  results->AppendInteger(*main_routing_id);
  info->PostResult(results.Pass());
}

}  // namespace xwalk
