// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_NAVIGATION_TRACKER_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_NAVIGATION_TRACKER_H_

#include <set>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "chrome/test/chromedriver/chrome/devtools_event_listener.h"
#include "chrome/test/chromedriver/chrome/status.h"

namespace base {
class DictionaryValue;
}

class DevToolsClient;
class Status;

// Tracks the navigation state of the page.
class NavigationTracker : public DevToolsEventListener {
 public:
  enum LoadingState {
    kUnknown,
    kLoading,
    kNotLoading,
  };

  explicit NavigationTracker(DevToolsClient* client);
  NavigationTracker(DevToolsClient* client, LoadingState known_state);
  virtual ~NavigationTracker();

  // Gets whether a navigation is pending for the specified frame. |frame_id|
  // may be empty to signify the main frame.
  Status IsPendingNavigation(const std::string& frame_id, bool* is_pending);

  // Overridden from DevToolsEventListener:
  virtual Status OnConnected(DevToolsClient* client) OVERRIDE;
  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE;
  virtual Status OnCommandSuccess(DevToolsClient* client,
                                  const std::string& method) OVERRIDE;

 private:
  DevToolsClient* client_;
  LoadingState loading_state_;
  std::set<std::string> scheduled_frame_set_;

  DISALLOW_COPY_AND_ASSIGN(NavigationTracker);
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_NAVIGATION_TRACKER_H_
