// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/event_target.h"

#include "xwalk/jsapi/sysapps_common.h"

using namespace xwalk::jsapi::sysapps_common; // NOLINT

namespace xwalk {
namespace sysapps {

EventTarget::EventTarget() {
  RegisterFunction("addEventListener",
                   &EventTarget::OnAddEventListener);
  RegisterFunction("removeEventListener",
                   &EventTarget::OnRemoveEventListener);
}

EventTarget::~EventTarget() {}

void EventTarget::DispatchEvent(const std::string& type) {
  DispatchEvent(type, make_scoped_ptr(new base::ListValue));
}

void EventTarget::DispatchEvent(const std::string& type,
                                scoped_ptr<base::ListValue> data) {
  EventMap::iterator it = events_.find(type);
  if (it == events_.end())
    return;

  PostResult(it->second, data.Pass());
}

void EventTarget::OnAddEventListener(const FunctionInfo& info) {
  scoped_ptr<AddEventListener::Params>
      params(AddEventListener::Params::Create(*info.arguments));

  if (events_.find(params->type) != events_.end()) {
    LOG(WARNING) << "Trying to re-add the event '" << params->type << "'. "
        "This should be optized in the JavaScript side so this message is sent "
        "only once.";
    return;
  }

  events_[params->type] = info.callback_id;
  StartEvent(params->type);
}

void EventTarget::OnRemoveEventListener(const FunctionInfo& info) {
  scoped_ptr<RemoveEventListener::Params>
      params(RemoveEventListener::Params::Create(*info.arguments));

  EventMap::iterator it = events_.find(params->type);
  if (it == events_.end()) {
    LOG(WARNING) << "Attempt to remove the event '" << params->type << "' but "
        "this event was not previously added.";
    return;
  }

  events_.erase(it);
  StopEvent(params->type);
}

}  // namespace sysapps
}  // namespace xwalk
