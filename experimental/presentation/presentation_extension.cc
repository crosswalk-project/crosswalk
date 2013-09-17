// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_extension.h"

#include <string>

#include "base/json/json_writer.h"
#include "base/memory/scoped_ptr.h"
#include "base/stl_util.h"
#include "base/values.h"
#include "content/public/browser/browser_thread.h"
#include "ipc/ipc_message.h"
#include "xwalk/experimental/presentation/presentation_display_manager.h"

// This will be generated from presentation_api.js.
extern const char kSource_presentation_api[];

namespace xwalk {
namespace experimental {

namespace {

const char kQueryDisplayAvailabilityCmd[] = "QueryDisplayAvailability";

}

using content::BrowserThread;
using extensions::XWalkExtension;
using extensions::XWalkExtensionInstance;

PresentationExtension::PresentationExtension()
  : display_available_(false) {
  set_name("navigator.experimental.presentation");

  display_manager_.reset(new PresentationDisplayManager);
  display_manager_->EnsureInitialized();
  display_manager_->AddObserver(this);
}

PresentationExtension::~PresentationExtension() {
  display_manager_->RemoveObserver(this);
  display_manager_.reset();
}

const char* PresentationExtension::GetJavaScriptAPI() {
  return kSource_presentation_api;
}

XWalkExtensionInstance* PresentationExtension::CreateInstance() {
  return new PresentationInstance(this);
}

void PresentationExtension::AddInstance(PresentationInstance* instance) {
  instance_list_.push_back(instance);
}

void PresentationExtension::RemoveInstance(PresentationInstance* instance) {
  std::vector<PresentationInstance*>::iterator it;
  for (it = instance_list_.begin(); it != instance_list_.end(); ++it)
    if ((*it) == instance) break;

  if (it != instance_list_.end())
    instance_list_.erase(it);
}

void PresentationExtension::OnDisplayAvailabilityChanged(bool is_available) {
  if (display_available_ == is_available)
    return;

  display_available_ = is_available;

  for (std::vector<PresentationInstance*>::iterator it = instance_list_.begin();
      it != instance_list_.end(); ++it) {
    (*it)->OnDisplayAvailabilityChanged(display_available_);
  }
}

//////////////////////////////////////////////////////////////////
//  PresentationInstance
//////////////////////////////////////////////////////////////////
PresentationInstance::PresentationInstance(PresentationExtension* extension)
  : extension_(extension) {
  extension_->AddInstance(this);
}

PresentationInstance::~PresentationInstance() {
  extension_->RemoveInstance(this);
}

void PresentationInstance::OnDisplayAvailabilityChanged(bool is_available) {
  // Dispatch display availabe change to renderer process.
  base::DictionaryValue value;
  value.SetString("cmd", "DisplayAvailableChange");
  value.SetBoolean("data", is_available);

  // Convert the value to JSON-format string.
  std::string json;
  base::JSONWriter::Write(&value, &json);

  scoped_ptr<StringValue> message(new StringValue(json));
  PostMessageToJS(scoped_ptr<base::Value>(message.release()));
}

void PresentationInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  NOTIMPLEMENTED();
}

void PresentationInstance::HandleSyncMessage(scoped_ptr<base::Value> msg) {
  scoped_ptr<base::Value> response;
  std::string cmd;
  if (!msg->GetAsString(&cmd) && cmd != kQueryDisplayAvailabilityCmd) {
    LOG(WARNING) << "Invalid command received.";
    response.reset(base::Value::CreateNullValue());
  } else  {
    response.reset(base::Value::CreateStringValue(
            extension_->display_available() ? "true" : "false"));
  }

  SendSyncReplyToJS(response.Pass());
}

}  // namespace experimental
}  // namespace xwalk

