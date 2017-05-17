// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/performance_logger.h"

#include <list>
#include <string>

#include "base/compiler_specific.h"
#include "base/format_macros.h"
#include "base/json/json_reader.h"
#include "base/memory/scoped_vector.h"
#include "base/time/time.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/log.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/xwalk/stub_devtools_client.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class FakeDevToolsClient : public StubDevToolsClient {
 public:
  explicit FakeDevToolsClient(const std::string& id)
      : id_(id), listener_(NULL) {}
  virtual ~FakeDevToolsClient() {}

  std::string PopSentCommand() {
    std::string command;
    if (!sent_command_queue_.empty()) {
      command = sent_command_queue_.front();
      sent_command_queue_.pop_front();
    }
    return command;
  }

  Status TriggerEvent(const std::string& method) {
    base::DictionaryValue empty_params;
    return listener_->OnEvent(this, method, empty_params);
  }

  // Overridden from DevToolsClient:
  virtual Status ConnectIfNecessary() OVERRIDE {
    return listener_->OnConnected(this);
  }

  virtual Status SendCommandAndGetResult(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result) OVERRIDE {
    sent_command_queue_.push_back(method);
    return Status(kOk);
  }

  virtual void AddListener(DevToolsEventListener* listener) OVERRIDE {
    CHECK(!listener_);
    listener_ = listener;
  }

  virtual const std::string& GetId() OVERRIDE {
    return id_;
  }

 private:
  const std::string id_;  // WebView id.
  std::list<std::string> sent_command_queue_;  // Commands that were sent.
  DevToolsEventListener* listener_;  // The fake allows only one event listener.
};

struct LogEntry {
  const base::Time timestamp;
  const Log::Level level;
  const std::string source;
  const std::string message;

  LogEntry(const base::Time& timestamp,
           Log::Level level,
           const std::string& source,
           const std::string& message)
      : timestamp(timestamp), level(level), source(source), message(message) {}
};

class FakeLog : public Log {
 public:
  virtual void AddEntryTimestamped(const base::Time& timestamp,
                        Level level,
                        const std::string& source,
                        const std::string& message) OVERRIDE;

  const ScopedVector<LogEntry>& GetEntries() {
    return entries_;
  }

 private:
  ScopedVector<LogEntry> entries_;
};

void FakeLog::AddEntryTimestamped(const base::Time& timestamp,
                                  Level level,
                                  const std::string& source,
                                  const std::string& message) {
  entries_.push_back(new LogEntry(timestamp, level, source, message));
}

scoped_ptr<DictionaryValue> ParseDictionary(const std::string& json) {
  std::string error;
  scoped_ptr<Value> value(base::JSONReader::ReadAndReturnError(
      json, base::JSON_PARSE_RFC, NULL, &error));
  if (value == NULL) {
    SCOPED_TRACE(json.c_str());
    SCOPED_TRACE(error.c_str());
    ADD_FAILURE();
    return scoped_ptr<DictionaryValue>();
  }
  DictionaryValue* dict = NULL;
  if (!value->GetAsDictionary(&dict)) {
    SCOPED_TRACE("JSON object is not a dictionary");
    ADD_FAILURE();
    return scoped_ptr<DictionaryValue>();
  }
  return scoped_ptr<DictionaryValue>(dict->DeepCopy());
}

void ValidateLogEntry(const LogEntry *entry,
                      const std::string& expected_webview,
                      const std::string& expected_method) {
  EXPECT_EQ(Log::kInfo, entry->level);
  EXPECT_LT(0, entry->timestamp.ToTimeT());

  scoped_ptr<base::DictionaryValue> message(ParseDictionary(entry->message));
  std::string webview;
  EXPECT_TRUE(message->GetString("webview", &webview));
  EXPECT_EQ(expected_webview, webview);
  std::string method;
  EXPECT_TRUE(message->GetString("message.method", &method));
  EXPECT_EQ(expected_method, method);
  DictionaryValue* params;
  EXPECT_TRUE(message->GetDictionary("message.params", &params));
  EXPECT_EQ(0u, params->size());
}

void ExpectEnableDomains(FakeDevToolsClient& client) {  // NOLINT
  EXPECT_EQ("Network.enable", client.PopSentCommand());
  EXPECT_EQ("Page.enable", client.PopSentCommand());
  EXPECT_EQ("Timeline.start", client.PopSentCommand());
  EXPECT_TRUE(client.PopSentCommand().empty());
}

}  // namespace

TEST(PerformanceLogger, OneWebView) {
  FakeDevToolsClient client("webview-1");
  FakeLog log;
  PerformanceLogger logger(&log);

  client.AddListener(&logger);
  logger.OnConnected(&client);
  ExpectEnableDomains(client);
  ASSERT_EQ(kOk, client.TriggerEvent("Network.gaga").code());
  ASSERT_EQ(kOk, client.TriggerEvent("Page.ulala").code());
  // Ignore -- different domain.
  ASSERT_EQ(kOk, client.TriggerEvent("Console.bad").code());

  ASSERT_EQ(2u, log.GetEntries().size());
  ValidateLogEntry(log.GetEntries()[0], "webview-1", "Network.gaga");
  ValidateLogEntry(log.GetEntries()[1], "webview-1", "Page.ulala");
}

TEST(PerformanceLogger, TwoWebViews) {
  FakeDevToolsClient client1("webview-1");
  FakeDevToolsClient client2("webview-2");
  FakeLog log;
  PerformanceLogger logger(&log);

  client1.AddListener(&logger);
  client2.AddListener(&logger);
  logger.OnConnected(&client1);
  logger.OnConnected(&client2);
  ExpectEnableDomains(client1);
  ExpectEnableDomains(client2);
  // OnConnected sends the enable command only to that client, not others.
  client1.ConnectIfNecessary();
  ExpectEnableDomains(client1);
  EXPECT_TRUE(client2.PopSentCommand().empty());;

  ASSERT_EQ(kOk, client1.TriggerEvent("Page.gaga1").code());
  ASSERT_EQ(kOk, client2.TriggerEvent("Timeline.gaga2").code());

  ASSERT_EQ(2u, log.GetEntries().size());
  ValidateLogEntry(log.GetEntries()[0], "webview-1", "Page.gaga1");
  ValidateLogEntry(log.GetEntries()[1], "webview-2", "Timeline.gaga2");
}
