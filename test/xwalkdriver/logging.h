// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_LOGGING_H_
#define XWALK_TEST_XWALKDRIVER_LOGGING_H_

#include <string>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/log.h"

struct Capabilities;
class DevToolsEventListener;
class ListValue;
class Status;

// Accumulates WebDriver Logging API entries of a given type and minimum level.
// See https://code.google.com/p/selenium/wiki/Logging.
class WebDriverLog : public Log {
 public:
  static const char kBrowserType[];
  static const char kDriverType[];
  static const char kPerformanceType[];

  // Converts WD wire protocol level name -> Level, false on bad name.
  static bool NameToLevel(const std::string& name, Level* out_level);

  // Creates a WebDriverLog with the given type and minimum level.
  WebDriverLog(const std::string& type, Level min_level);
  virtual ~WebDriverLog();

  // Returns entries accumulated so far, as a ListValue ready for serialization
  // into the wire protocol response to the "/log" command.
  // The caller assumes ownership of the ListValue, and the WebDriverLog
  // creates and owns a new empty ListValue for further accumulation.
  scoped_ptr<base::ListValue> GetAndClearEntries();

  // Translates a Log entry level into a WebDriver level and stores the entry.
  virtual void AddEntryTimestamped(const base::Time& timestamp,
                                   Level level,
                                   const std::string& source,
                                   const std::string& message) OVERRIDE;

  const std::string& type() const;
  void set_min_level(Level min_level);
  Level min_level() const;

 private:
  const std::string type_;  // WebDriver log type.
  Level min_level_;  // Minimum level of entries to store.
  scoped_ptr<base::ListValue> entries_;  // Accumulated entries.

  DISALLOW_COPY_AND_ASSIGN(WebDriverLog);
};

// Initializes logging system for XwalkDriver. Returns true on success.
bool InitLogging();

// Creates Log's and DevToolsEventListener's based on logging preferences.
Status CreateLogs(const Capabilities& capabilities,
                  ScopedVector<WebDriverLog>* out_logs,
                  ScopedVector<DevToolsEventListener>* out_listeners);

#endif  // XWALK_TEST_XWALKDRIVER_LOGGING_H_
