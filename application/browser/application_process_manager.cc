// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_process_manager.h"

#include <string>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/application_messages.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "net/base/net_util.h"

using xwalk::Runtime;
using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

ApplicationProcessManager::ApplicationProcessManager(
    RuntimeContext* runtime_context)
    : runtime_context_(runtime_context),
      runtime_counts_(0),
      main_runtime_(NULL),
      weak_ptr_factory_(this) {
  main_document_idle_time_ = base::TimeDelta::FromSeconds(10);
  main_document_suspending_time_ = base::TimeDelta::FromSeconds(5);
  unsigned idle_time_sec = 0;
  unsigned suspending_time_sec = 0;
  if (base::StringToUint(CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          switches::kMainDocumentIdleTime), &idle_time_sec)) {
    main_document_idle_time_ = base::TimeDelta::FromSeconds(idle_time_sec);
  }
  if (base::StringToUint(CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          switches::kMainDocumentSuspendingTime), &suspending_time_sec)) {
    main_document_suspending_time_ = base::TimeDelta::FromSeconds(
        suspending_time_sec);
  }
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

void ApplicationProcessManager::OnShouldSuspendAck(int sequence_id) {
  if (sequence_id == main_document_data_.close_sequence_id) {
    main_runtime_->web_contents()->GetRenderViewHost()->Send(
      new ApplicationMsg_Suspend());
  }
}

void ApplicationProcessManager::OnSuspendAck() {
  LOG(INFO) << "The main document is suspending and about to close.";
  main_document_data_.is_closing = true;
  base::MessageLoop::current()->PostDelayedTask(
    FROM_HERE,
    base::Bind(&ApplicationProcessManager::CloseMainDocumentNow,
               weak_ptr_factory_.GetWeakPtr(),
               main_document_data_.close_sequence_id),
    main_document_suspending_time_);
}

void ApplicationProcessManager::CancelSuspend() {
  LOG(INFO) << "Cancel suspending state of main document.";
  main_document_data_.is_closing = false;
  main_runtime_->web_contents()->GetRenderViewHost()->Send(
      new ApplicationMsg_CancelSuspend());
}

void ApplicationProcessManager::OnRuntimeAdded(Runtime* runtime) {
  ++runtime_counts_;
  // As long as a new Runtime object is added, the main document should
  // keep alive.
  OnMainDocumentActive();
}

void ApplicationProcessManager::OnRuntimeRemoved(Runtime* runtime) {
  --runtime_counts_;
  // If main document is not closing and is the last running runtime instance,
  // it should be closed.
  if (main_runtime_ && runtime_counts_ == 1 &&
      !main_document_data_.is_closing) {
    base::MessageLoop::current()->PostDelayedTask(
        FROM_HERE,
        base::Bind(&ApplicationProcessManager::OnMainDocumentIdle,
                   weak_ptr_factory_.GetWeakPtr(),
                   ++main_document_data_.close_sequence_id),
        main_document_idle_time_);
  }
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
  return true;
}

void ApplicationProcessManager::CloseMainDocumentNow(int sequence_id) {
  if (sequence_id == main_document_data_.close_sequence_id &&
      main_document_data_.is_closing &&
      main_runtime_) {
    main_runtime_->Close();
    LOG(INFO) << "The main document is closed now.";
  }
}

void ApplicationProcessManager::OnMainDocumentActive() {
  if (!main_document_data_.is_closing) {
    // Cancel the current close sequence id by changing close_sequence_id,
    // which causes to ignore the next ShouldSuspendAck.
    ++main_document_data_.close_sequence_id;
  } else {
    // If the main document is about to close, we should cancel suspend event.
    CancelSuspend();
  }
}

void ApplicationProcessManager::OnMainDocumentIdle(int sequence_id) {
  main_runtime_->web_contents()->GetRenderViewHost()->Send(
    new ApplicationMsg_ShouldSuspend(sequence_id));
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
