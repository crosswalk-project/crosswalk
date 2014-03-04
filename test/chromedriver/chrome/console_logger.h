// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_CONSOLE_LOGGER_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_CONSOLE_LOGGER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "chrome/test/chromedriver/chrome/devtools_event_listener.h"

class Log;

// Translates DevTools Console.messageAdded events into Log messages.
//
// The message format, in terms of Console.ConsoleMessage fields, is:
// "<url or source> [<line>[:<column>]] text"
//
// Translates the level into Log::Level, drops all other fields.
class ConsoleLogger : public DevToolsEventListener {
 public:
  // Creates a ConsoleLogger that creates entries in the given Log object.
  // The log is owned elsewhere and must not be null.
  explicit ConsoleLogger(Log* log);

  // Enables Console events for the client, which must not be null.
  virtual Status OnConnected(DevToolsClient* client) OVERRIDE;
  // Translates an event into a log entry.
  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE;

 private:
  Log* log_;  // The log where to create entries.

  DISALLOW_COPY_AND_ASSIGN(ConsoleLogger);
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_CONSOLE_LOGGER_H_
