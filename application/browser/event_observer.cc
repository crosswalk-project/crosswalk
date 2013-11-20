// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/event_observer.h"

#include "xwalk/application/browser/application_event_manager.h"

namespace xwalk {
namespace application {

EventObserver::EventObserver(ApplicationEventManager* event_manager)
  : event_manager_(event_manager) {
}

EventObserver::~EventObserver() {
  event_manager_->DetachObserver(this);
}

}  // namespace application
}  // namespace xwalk
