// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_DEVTOOLS_CLIENT_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_DEVTOOLS_CLIENT_H_

#include <string>

#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"

namespace base {
class DictionaryValue;
class TimeDelta;
}

class DevToolsEventListener;
class Status;

// A DevTools client of a single DevTools debugger.
class DevToolsClient {
 public:
  typedef base::Callback<Status(bool* is_condition_met)> ConditionalFunc;

  virtual ~DevToolsClient() {}

  virtual const std::string& GetId() = 0;

  virtual bool WasCrashed() = 0;

  // Connect to DevTools if the DevToolsClient is disconnected.
  virtual Status ConnectIfNecessary() = 0;

  virtual Status SendCommand(const std::string& method,
                             const base::DictionaryValue& params) = 0;
  virtual Status SendCommandAndGetResult(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result) = 0;

  // Adds a listener. This must only be done when the client is disconnected.
  virtual void AddListener(DevToolsEventListener* listener) = 0;

  // Handles events until the given function reports the condition is met
  // and there are no more received events to handle. If the given
  // function ever returns an error, returns immediately with the error.
  // If the condition is not met within |timeout|, kTimeout status
  // is returned eventually. If |timeout| is 0, this function will not block.
  virtual Status HandleEventsUntil(const ConditionalFunc& conditional_func,
                                   const base::TimeDelta& timeout) = 0;

  // Handles events that have been received but not yet handled.
  virtual Status HandleReceivedEvents() = 0;
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_DEVTOOLS_CLIENT_H_
