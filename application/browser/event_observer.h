// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_EVENT_OBSERVER_H_
#define XWALK_APPLICATION_BROWSER_EVENT_OBSERVER_H_

#include <string>

#include "base/callback.h"
#include "base/memory/ref_counted.h"

namespace xwalk {
namespace application {

class ApplicationEventManager;
class Event;

// Abstract base class to observe application events.
class EventObserver {
 public:
  explicit EventObserver(ApplicationEventManager* event_namager);

  virtual void Observe(const std::string& app_id,
                       scoped_refptr<Event> event) = 0;

 protected:
  virtual ~EventObserver();
  ApplicationEventManager* event_manager_;
};

}  // namespace application
}  // namespace xwalk
#endif  // XWALK_APPLICATION_BROWSER_EVENT_OBSERVER_H_
