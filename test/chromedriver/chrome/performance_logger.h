// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_PERFORMANCE_LOGGER_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_PERFORMANCE_LOGGER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "xwalk/test/chromedriver/chrome/devtools_event_listener.h"

class Log;

// Translates DevTools profiler events into Log messages with info level.
//
// The message is a JSON string of the following structure:
// {
//    "webview": <originating WebView ID>,
//    "message": { "method": "...", "params": { ... }}  // DevTools message.
// }
class PerformanceLogger : public DevToolsEventListener {
 public:
  // Creates a PerformanceLogger that creates entries in the given Log object.
  // The log is owned elsewhere and must not be null.
  explicit PerformanceLogger(Log* log);

  // Enables Page,Network,Timeline events for client, which must not be null.
  virtual Status OnConnected(DevToolsClient* client) OVERRIDE;
  // Translates an event into a log entry.
  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE;

 private:
  Log* log_;  // The log where to create entries.

  DISALLOW_COPY_AND_ASSIGN(PerformanceLogger);
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_PERFORMANCE_LOGGER_H_
