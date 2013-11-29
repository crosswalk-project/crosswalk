// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_process_manager.h"

#include <string>

#include "base/stl_util.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::Runtime;
using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

ApplicationProcessManager::ApplicationProcessManager(
    RuntimeContext* runtime_context)
    : runtime_context_(runtime_context),
      main_runtime_(NULL),
      weak_ptr_factory_(this) {
}

ApplicationProcessManager::~ApplicationProcessManager() {
}

bool ApplicationProcessManager::LaunchApplication(
        RuntimeContext* runtime_context,
        const Application* application) {
  if (RunMainDocument(application))
    return true;
  // NOTE: For now we allow launching a web app from a local path. This may go
  // away at some point.
  return RunFromLocalPath(application);
}

void ApplicationProcessManager::OnRuntimeAdded(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.insert(runtime);
}

void ApplicationProcessManager::OnRuntimeRemoved(Runtime* runtime) {
  DCHECK(runtime);
  runtimes_.erase(runtime);
  // FIXME: main_runtime_ should always be the last one to close
  // in RuntimeRegistry. Need to fix the issue from browser tests.
  if (runtimes_.size() == 1 &&
      ContainsKey(runtimes_, main_runtime_))
    CloseMainDocument();
}

bool ApplicationProcessManager::RunMainDocument(
    const Application* application) {
  const Manifest* manifest = application->GetManifest();
  const base::DictionaryValue* dict = NULL;
  if (!manifest->GetDictionary(application_manifest_keys::kAppMainKey, &dict))
    return false;

  GURL url;
  std::string main_source;
  const base::ListValue* main_scripts = NULL;
  manifest->GetString(application_manifest_keys::kAppMainSourceKey,
      &main_source);
  manifest->GetList(application_manifest_keys::kAppMainScriptsKey,
      &main_scripts);

  if (!main_source.empty() && (main_scripts && main_scripts->GetSize()))
    LOG(WARNING) << "An app should not has more than one main document.";

  if (!main_source.empty()) {
    url = application->GetResourceURL(main_source);
  } else if (main_scripts && main_scripts->GetSize()) {
    // When no main.source is defined but main.scripts are, we implicitly create
    // a main document.
    url = application->GetResourceURL(kGeneratedMainDocumentFilename);
  } else {
    LOG(WARNING) << "The app.main field doesn't contain a valid main document.";
    return false;
  }

  main_runtime_ = Runtime::Create(runtime_context_, url);
  ApplicationEventManager* event_manager =
      runtime_context_->GetApplicationSystem()->event_manager();
  event_manager->OnMainDocumentCreated(
      application->ID(), main_runtime_->web_contents());
  return true;
}

void ApplicationProcessManager::CloseMainDocument() {
  DCHECK(main_runtime_);
  main_runtime_->Close();
  main_runtime_ = NULL;
}

bool ApplicationProcessManager::RunFromLocalPath(
    const Application* application) {
  const Manifest* manifest = application->GetManifest();
  std::string entry_page;
  if (manifest->GetString(application_manifest_keys::kLaunchLocalPathKey,
        &entry_page) && !entry_page.empty()) {
    GURL url = application->GetResourceURL(entry_page);
    if (url.is_empty()) {
      LOG(WARNING) << "Can't find a valid local path URL for app.";
      return false;
    }

    Runtime::CreateWithDefaultWindow(runtime_context_, url);
    return true;
  }

  return false;
}

}  // namespace application
}  // namespace xwalk
