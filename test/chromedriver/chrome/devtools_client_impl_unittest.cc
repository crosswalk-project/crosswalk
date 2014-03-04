// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>
#include <string>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/devtools_client_impl.h"
#include "chrome/test/chromedriver/chrome/devtools_event_listener.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/net/sync_websocket.h"
#include "chrome/test/chromedriver/net/sync_websocket_factory.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace {

Status CloserFunc() {
  return Status(kOk);
}

class MockSyncWebSocket : public SyncWebSocket {
 public:
  MockSyncWebSocket() : connected_(false), id_(-1), queued_messages_(1) {}
  virtual ~MockSyncWebSocket() {}

  virtual bool IsConnected() OVERRIDE {
    return connected_;
  }

  virtual bool Connect(const GURL& url) OVERRIDE {
    EXPECT_STREQ("http://url/", url.possibly_invalid_spec().c_str());
    connected_ = true;
    return true;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    EXPECT_TRUE(connected_);
    scoped_ptr<base::Value> value(base::JSONReader::Read(message));
    base::DictionaryValue* dict = NULL;
    EXPECT_TRUE(value->GetAsDictionary(&dict));
    if (!dict)
      return false;
    EXPECT_TRUE(dict->GetInteger("id", &id_));
    std::string method;
    EXPECT_TRUE(dict->GetString("method", &method));
    EXPECT_STREQ("method", method.c_str());
    base::DictionaryValue* params = NULL;
    EXPECT_TRUE(dict->GetDictionary("params", &params));
    if (!params)
      return false;
    int param = -1;
    EXPECT_TRUE(params->GetInteger("param", &param));
    EXPECT_EQ(1, param);
    return true;
  }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    if (timeout <= base::TimeDelta())
      return SyncWebSocket::kTimeout;
    base::DictionaryValue response;
    response.SetInteger("id", id_);
    base::DictionaryValue result;
    result.SetInteger("param", 1);
    response.Set("result", result.DeepCopy());
    base::JSONWriter::Write(&response, message);
    --queued_messages_;
    return SyncWebSocket::kOk;
  }

  virtual bool HasNextMessage() OVERRIDE {
    return queued_messages_ > 0;
  }

 protected:
  bool connected_;
  int id_;
  int queued_messages_;
};

template <typename T>
scoped_ptr<SyncWebSocket> CreateMockSyncWebSocket() {
  return scoped_ptr<SyncWebSocket>(new T());
}

class DevToolsClientImplTest : public testing::Test {
 protected:
  DevToolsClientImplTest() : long_timeout_(base::TimeDelta::FromMinutes(5)) {}

  const base::TimeDelta long_timeout_;
};

}  // namespace

TEST_F(DevToolsClientImplTest, SendCommand) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  params.SetInteger("param", 1);
  ASSERT_EQ(kOk, client.SendCommand("method", params).code());
}

TEST_F(DevToolsClientImplTest, SendCommandAndGetResult) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  params.SetInteger("param", 1);
  scoped_ptr<base::DictionaryValue> result;
  Status status = client.SendCommandAndGetResult("method", params, &result);
  ASSERT_EQ(kOk, status.code());
  std::string json;
  base::JSONWriter::Write(result.get(), &json);
  ASSERT_STREQ("{\"param\":1}", json.c_str());
}

namespace {

class MockSyncWebSocket2 : public SyncWebSocket {
 public:
  MockSyncWebSocket2() {}
  virtual ~MockSyncWebSocket2() {}

  virtual bool IsConnected() OVERRIDE {
    return false;
  }

  virtual bool Connect(const GURL& url) OVERRIDE {
    return false;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    EXPECT_TRUE(false);
    return false;
  }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    EXPECT_TRUE(false);
    return SyncWebSocket::kDisconnected;
  }

  virtual bool HasNextMessage() OVERRIDE {
    return true;
  }
};

}  // namespace

TEST_F(DevToolsClientImplTest, ConnectIfNecessaryConnectFails) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket2>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc));
  ASSERT_EQ(kDisconnected, client.ConnectIfNecessary().code());
}

namespace {

class MockSyncWebSocket3 : public SyncWebSocket {
 public:
  MockSyncWebSocket3() : connected_(false) {}
  virtual ~MockSyncWebSocket3() {}

  virtual bool IsConnected() OVERRIDE {
    return connected_;
  }

  virtual bool Connect(const GURL& url) OVERRIDE {
    connected_ = true;
    return true;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    return false;
  }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    EXPECT_TRUE(false);
    return SyncWebSocket::kDisconnected;
  }

  virtual bool HasNextMessage() OVERRIDE {
    return true;
  }

 private:
  bool connected_;
};

}  // namespace

TEST_F(DevToolsClientImplTest, SendCommandSendFails) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket3>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  ASSERT_TRUE(client.SendCommand("method", params).IsError());
}

namespace {

class MockSyncWebSocket4 : public SyncWebSocket {
 public:
  MockSyncWebSocket4() : connected_(false) {}
  virtual ~MockSyncWebSocket4() {}

  virtual bool IsConnected() OVERRIDE {
    return connected_;
  }

  virtual bool Connect(const GURL& url) OVERRIDE {
    connected_ = true;
    return true;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    return true;
  }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    return SyncWebSocket::kDisconnected;
  }

  virtual bool HasNextMessage() OVERRIDE {
    return true;
  }

 private:
  bool connected_;
};

}  // namespace

TEST_F(DevToolsClientImplTest, SendCommandReceiveNextMessageFails) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket4>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  ASSERT_TRUE(client.SendCommand("method", params).IsError());
}

namespace {

class FakeSyncWebSocket : public SyncWebSocket {
 public:
  FakeSyncWebSocket() : connected_(false) {}
  virtual ~FakeSyncWebSocket() {}

  virtual bool IsConnected() OVERRIDE {
    return connected_;
  }

  virtual bool Connect(const GURL& url) OVERRIDE {
    EXPECT_FALSE(connected_);
    connected_ = true;
    return true;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    return true;
  }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    return SyncWebSocket::kOk;
  }

  virtual bool HasNextMessage() OVERRIDE {
    return true;
  }

 private:
  bool connected_;
};

bool ReturnCommand(
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  *type = internal::kCommandResponseMessageType;
  command_response->id = expected_id;
  command_response->result.reset(new base::DictionaryValue());
  return true;
}

bool ReturnBadResponse(
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  *type = internal::kCommandResponseMessageType;
  command_response->id = expected_id;
  command_response->result.reset(new base::DictionaryValue());
  return false;
}

bool ReturnCommandBadId(
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  *type = internal::kCommandResponseMessageType;
  command_response->id = expected_id + 100;
  command_response->result.reset(new base::DictionaryValue());
  return true;
}

bool ReturnCommandError(
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  *type = internal::kCommandResponseMessageType;
  command_response->id = expected_id;
  command_response->error = "err";
  return true;
}

class MockListener : public DevToolsEventListener {
 public:
  MockListener() : called_(false) {}
  virtual ~MockListener() {
    EXPECT_TRUE(called_);
  }

  virtual Status OnConnected(DevToolsClient* client) OVERRIDE {
    return Status(kOk);
  }

  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE {
    called_ = true;
    EXPECT_STREQ("method", method.c_str());
    EXPECT_TRUE(params.HasKey("key"));
    return Status(kOk);
  }

 private:
  bool called_;
};

bool ReturnEventThenResponse(
    bool* first,
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  if (*first) {
    *type = internal::kEventMessageType;
    event->method = "method";
    event->params.reset(new base::DictionaryValue());
    event->params->SetInteger("key", 1);
  } else {
    *type = internal::kCommandResponseMessageType;
    command_response->id = expected_id;
    base::DictionaryValue params;
    command_response->result.reset(new base::DictionaryValue());
    command_response->result->SetInteger("key", 2);
  }
  *first = false;
  return true;
}

bool ReturnEvent(
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  *type = internal::kEventMessageType;
  event->method = "method";
  event->params.reset(new base::DictionaryValue());
  event->params->SetInteger("key", 1);
  return true;
}

bool ReturnOutOfOrderResponses(
    int* recurse_count,
    DevToolsClient* client,
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  int key = 0;
  base::DictionaryValue params;
  params.SetInteger("param", 1);
  switch ((*recurse_count)++) {
    case 0:
      client->SendCommand("method", params);
      *type = internal::kEventMessageType;
      event->method = "method";
      event->params.reset(new base::DictionaryValue());
      event->params->SetInteger("key", 1);
      return true;
    case 1:
      command_response->id = expected_id - 1;
      key = 2;
      break;
    case 2:
      command_response->id = expected_id;
      key = 3;
      break;
  }
  *type = internal::kCommandResponseMessageType;
  command_response->result.reset(new base::DictionaryValue());
  command_response->result->SetInteger("key", key);
  return true;
}

bool ReturnError(
    const std::string& message,
    int expected_id,
    internal::InspectorMessageType* type,
    internal::InspectorEvent* event,
    internal::InspectorCommandResponse* command_response) {
  return false;
}

Status AlwaysTrue(bool* is_met) {
  *is_met = true;
  return Status(kOk);
}

Status AlwaysError(bool* is_met) {
  return Status(kUnknownError);
}

}  // namespace

TEST_F(DevToolsClientImplTest, SendCommandOnlyConnectsOnce) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<FakeSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnCommand));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  ASSERT_TRUE(client.SendCommand("method", params).IsOk());
  ASSERT_TRUE(client.SendCommand("method", params).IsOk());
}

TEST_F(DevToolsClientImplTest, SendCommandBadResponse) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<FakeSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnBadResponse));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  ASSERT_TRUE(client.SendCommand("method", params).IsError());
}

TEST_F(DevToolsClientImplTest, SendCommandBadId) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<FakeSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnCommandBadId));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  ASSERT_TRUE(client.SendCommand("method", params).IsError());
}

TEST_F(DevToolsClientImplTest, SendCommandResponseError) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<FakeSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnCommandError));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  ASSERT_TRUE(client.SendCommand("method", params).IsError());
}

TEST_F(DevToolsClientImplTest, SendCommandEventBeforeResponse) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<FakeSyncWebSocket>);
  MockListener listener;
  bool first = true;
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnEventThenResponse, &first));
  client.AddListener(&listener);
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  scoped_ptr<base::DictionaryValue> result;
  ASSERT_TRUE(client.SendCommandAndGetResult("method", params, &result).IsOk());
  ASSERT_TRUE(result);
  int key;
  ASSERT_TRUE(result->GetInteger("key", &key));
  ASSERT_EQ(2, key);
}

TEST(ParseInspectorMessage, NonJson) {
  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  ASSERT_FALSE(internal::ParseInspectorMessage(
      "hi", 0, &type, &event, &response));
}

TEST(ParseInspectorMessage, NeitherCommandNorEvent) {
  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  ASSERT_FALSE(internal::ParseInspectorMessage(
      "{}", 0, &type, &event, &response));
}

TEST(ParseInspectorMessage, EventNoParams) {
  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  ASSERT_TRUE(internal::ParseInspectorMessage(
      "{\"method\":\"method\"}", 0, &type, &event, &response));
  ASSERT_EQ(internal::kEventMessageType, type);
  ASSERT_STREQ("method", event.method.c_str());
  ASSERT_TRUE(event.params->IsType(base::Value::TYPE_DICTIONARY));
}

TEST(ParseInspectorMessage, EventWithParams) {
  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  ASSERT_TRUE(internal::ParseInspectorMessage(
      "{\"method\":\"method\",\"params\":{\"key\":100}}",
      0, &type, &event, &response));
  ASSERT_EQ(internal::kEventMessageType, type);
  ASSERT_STREQ("method", event.method.c_str());
  int key;
  ASSERT_TRUE(event.params->GetInteger("key", &key));
  ASSERT_EQ(100, key);
}

TEST(ParseInspectorMessage, CommandNoErrorOrResult) {
  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  ASSERT_FALSE(internal::ParseInspectorMessage(
      "{\"id\":1}", 0, &type, &event, &response));
}

TEST(ParseInspectorMessage, CommandError) {
  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  ASSERT_TRUE(internal::ParseInspectorMessage(
      "{\"id\":1,\"error\":{}}", 0, &type, &event, &response));
  ASSERT_EQ(internal::kCommandResponseMessageType, type);
  ASSERT_EQ(1, response.id);
  ASSERT_TRUE(response.error.length());
  ASSERT_FALSE(response.result);
}

TEST(ParseInspectorMessage, Command) {
  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  ASSERT_TRUE(internal::ParseInspectorMessage(
      "{\"id\":1,\"result\":{\"key\":1}}", 0, &type, &event, &response));
  ASSERT_EQ(internal::kCommandResponseMessageType, type);
  ASSERT_EQ(1, response.id);
  ASSERT_FALSE(response.error.length());
  int key;
  ASSERT_TRUE(response.result->GetInteger("key", &key));
  ASSERT_EQ(1, key);
}

TEST_F(DevToolsClientImplTest, HandleEventsUntil) {
  MockListener listener;
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnEvent));
  client.AddListener(&listener);
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  Status status = client.HandleEventsUntil(base::Bind(&AlwaysTrue),
                                           long_timeout_);
  ASSERT_EQ(kOk, status.code());
}

TEST_F(DevToolsClientImplTest, HandleEventsUntilTimeout) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnEvent));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  Status status = client.HandleEventsUntil(base::Bind(&AlwaysTrue),
                                           base::TimeDelta());
  ASSERT_EQ(kTimeout, status.code());
}

TEST_F(DevToolsClientImplTest, WaitForNextEventCommand) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnCommand));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  Status status = client.HandleEventsUntil(base::Bind(&AlwaysTrue),
                                           long_timeout_);
  ASSERT_EQ(kUnknownError, status.code());
}

TEST_F(DevToolsClientImplTest, WaitForNextEventError) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnError));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  Status status = client.HandleEventsUntil(base::Bind(&AlwaysTrue),
                                           long_timeout_);
  ASSERT_EQ(kUnknownError, status.code());
}

TEST_F(DevToolsClientImplTest, WaitForNextEventConditionalFuncReturnsError) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc),
                            base::Bind(&ReturnEvent));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  Status status = client.HandleEventsUntil(base::Bind(&AlwaysError),
                                           long_timeout_);
  ASSERT_EQ(kUnknownError, status.code());
}

TEST_F(DevToolsClientImplTest, NestedCommandsWithOutOfOrderResults) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket>);
  int recurse_count = 0;
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc));
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  client.SetParserFuncForTesting(
      base::Bind(&ReturnOutOfOrderResponses, &recurse_count, &client));
  base::DictionaryValue params;
  params.SetInteger("param", 1);
  scoped_ptr<base::DictionaryValue> result;
  ASSERT_TRUE(client.SendCommandAndGetResult("method", params, &result).IsOk());
  ASSERT_TRUE(result);
  int key;
  ASSERT_TRUE(result->GetInteger("key", &key));
  ASSERT_EQ(2, key);
}

namespace {

class OnConnectedListener : public DevToolsEventListener {
 public:
  OnConnectedListener(const std::string& method, DevToolsClient* client)
      : method_(method),
        client_(client),
        on_connected_called_(false),
        on_event_called_(false) {
    client_->AddListener(this);
  }
  virtual ~OnConnectedListener() {}

  void VerifyCalled() {
    EXPECT_TRUE(on_connected_called_);
    EXPECT_TRUE(on_event_called_);
  }

  virtual Status OnConnected(DevToolsClient* client) OVERRIDE {
    EXPECT_EQ(client_, client);
    EXPECT_STREQ("onconnected-id", client->GetId().c_str());
    EXPECT_FALSE(on_connected_called_);
    EXPECT_FALSE(on_event_called_);
    on_connected_called_ = true;
    base::DictionaryValue params;
    return client_->SendCommand(method_, params);
  }

  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE {
    EXPECT_EQ(client_, client);
    EXPECT_STREQ("onconnected-id", client->GetId().c_str());
    EXPECT_TRUE(on_connected_called_);
    on_event_called_ = true;
    return Status(kOk);
  }

 private:
  std::string method_;
  DevToolsClient* client_;
  bool on_connected_called_;
  bool on_event_called_;
};

class OnConnectedSyncWebSocket : public SyncWebSocket {
 public:
  OnConnectedSyncWebSocket() : connected_(false) {}
  virtual ~OnConnectedSyncWebSocket() {}

  virtual bool IsConnected() OVERRIDE {
    return connected_;
  }

  virtual bool Connect(const GURL& url) OVERRIDE {
    connected_ = true;
    return true;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    EXPECT_TRUE(connected_);
    scoped_ptr<base::Value> value(base::JSONReader::Read(message));
    base::DictionaryValue* dict = NULL;
    EXPECT_TRUE(value->GetAsDictionary(&dict));
    if (!dict)
      return false;
    int id;
    EXPECT_TRUE(dict->GetInteger("id", &id));
    std::string method;
    EXPECT_TRUE(dict->GetString("method", &method));

    base::DictionaryValue response;
    response.SetInteger("id", id);
    response.Set("result", new base::DictionaryValue());
    std::string json_response;
    base::JSONWriter::Write(&response, &json_response);
    queued_response_.push_back(json_response);

    // Push one event.
    base::DictionaryValue event;
    event.SetString("method", "updateEvent");
    event.Set("params", new base::DictionaryValue());
    std::string json_event;
    base::JSONWriter::Write(&event, &json_event);
    queued_response_.push_back(json_event);

    return true;
  }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    if (queued_response_.empty())
      return SyncWebSocket::kDisconnected;
    *message = queued_response_.front();
    queued_response_.pop_front();
    return SyncWebSocket::kOk;
  }

  virtual bool HasNextMessage() OVERRIDE {
    return !queued_response_.empty();
  }

 private:
  bool connected_;
  std::list<std::string> queued_response_;
};

}  // namespace

TEST_F(DevToolsClientImplTest, ProcessOnConnectedFirstOnCommand) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<OnConnectedSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "onconnected-id",
                            base::Bind(&CloserFunc));
  OnConnectedListener listener1("DOM.getDocument", &client);
  OnConnectedListener listener2("Runtime.enable", &client);
  OnConnectedListener listener3("Page.enable", &client);
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  base::DictionaryValue params;
  EXPECT_EQ(kOk, client.SendCommand("Runtime.execute", params).code());
  listener1.VerifyCalled();
  listener2.VerifyCalled();
  listener3.VerifyCalled();
}

TEST_F(DevToolsClientImplTest, ProcessOnConnectedFirstOnHandleEventsUntil) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<OnConnectedSyncWebSocket>);
  DevToolsClientImpl client(factory, "http://url", "onconnected-id",
                            base::Bind(&CloserFunc));
  OnConnectedListener listener1("DOM.getDocument", &client);
  OnConnectedListener listener2("Runtime.enable", &client);
  OnConnectedListener listener3("Page.enable", &client);
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  EXPECT_EQ(kOk, client.HandleReceivedEvents().code());
  listener1.VerifyCalled();
  listener2.VerifyCalled();
  listener3.VerifyCalled();
}

namespace {

class MockSyncWebSocket5 : public SyncWebSocket {
 public:
  MockSyncWebSocket5() : request_no_(0) {}
  virtual ~MockSyncWebSocket5() {}

  virtual bool IsConnected() OVERRIDE {
    return true;
  }

  virtual bool Connect(const GURL& url) OVERRIDE {
    return true;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    return true;
  }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    if (request_no_ == 0) {
      *message = "{\"method\": \"m\", \"params\": {}}";
    } else {
      *message = base::StringPrintf(
          "{\"result\": {}, \"id\": %d}", request_no_);
    }
    request_no_++;
    return SyncWebSocket::kOk;
  }

  virtual bool HasNextMessage() OVERRIDE {
    return false;
  }

 private:
  int request_no_;
};

class OtherEventListener : public DevToolsEventListener {
 public:
  OtherEventListener() : received_event_(false) {}
  virtual ~OtherEventListener() {}

  virtual Status OnConnected(DevToolsClient* client) OVERRIDE {
    return Status(kOk);
  }
  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE {
    received_event_ = true;
    return Status(kOk);
  }

  bool received_event_;
};

class OnEventListener : public DevToolsEventListener {
 public:
  OnEventListener(DevToolsClient* client,
                  OtherEventListener* other_listener)
      : client_(client),
        other_listener_(other_listener) {}
  virtual ~OnEventListener() {}

  virtual Status OnConnected(DevToolsClient* client) OVERRIDE {
    EXPECT_EQ(client_, client);
    return Status(kOk);
  }

  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE {
    EXPECT_EQ(client_, client);
    client_->SendCommand("method", params);
    EXPECT_TRUE(other_listener_->received_event_);
    return Status(kOk);
  }

 private:
  DevToolsClient* client_;
  OtherEventListener* other_listener_;
};

}  // namespace

TEST_F(DevToolsClientImplTest, ProcessOnEventFirst) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<MockSyncWebSocket5>);
  DevToolsClientImpl client(factory, "http://url", "id",
                            base::Bind(&CloserFunc));
  OtherEventListener listener2;
  OnEventListener listener1(&client, &listener2);
  client.AddListener(&listener1);
  client.AddListener(&listener2);
  base::DictionaryValue params;
  EXPECT_EQ(kOk, client.SendCommand("method", params).code());
}

namespace {

class DisconnectedSyncWebSocket : public MockSyncWebSocket {
 public:
  DisconnectedSyncWebSocket() : connection_count_(0), command_count_(0) {}
  virtual ~DisconnectedSyncWebSocket() {}

  virtual bool Connect(const GURL& url) OVERRIDE {
    connection_count_++;
    connected_ = connection_count_ != 2;
    return connected_;
  }

  virtual bool Send(const std::string& message) OVERRIDE {
    command_count_++;
    if (command_count_ == 1) {
      connected_ = false;
      return false;
    }
    return MockSyncWebSocket::Send(message);
  }

 private:
  int connection_count_;
  int command_count_;
};

Status CheckCloserFuncCalled(bool* is_called) {
  *is_called = true;
  return Status(kOk);
}

}  // namespace

TEST_F(DevToolsClientImplTest, Reconnect) {
  SyncWebSocketFactory factory =
      base::Bind(&CreateMockSyncWebSocket<DisconnectedSyncWebSocket>);
  bool is_called = false;
  DevToolsClientImpl client(factory,
                            "http://url",
                            "id",
                            base::Bind(&CheckCloserFuncCalled, &is_called));
  ASSERT_FALSE(is_called);
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  ASSERT_FALSE(is_called);
  base::DictionaryValue params;
  params.SetInteger("param", 1);
  is_called = false;
  ASSERT_EQ(kDisconnected, client.SendCommand("method", params).code());
  ASSERT_FALSE(is_called);
  ASSERT_EQ(kDisconnected, client.HandleReceivedEvents().code());
  ASSERT_FALSE(is_called);
  ASSERT_EQ(kOk, client.ConnectIfNecessary().code());
  ASSERT_TRUE(is_called);
  is_called = false;
  ASSERT_EQ(kOk, client.SendCommand("method", params).code());
  ASSERT_FALSE(is_called);
}

namespace {

class MockSyncWebSocket6 : public SyncWebSocket {
 public:
  explicit MockSyncWebSocket6(std::list<std::string>* messages)
      : messages_(messages) {}
  virtual ~MockSyncWebSocket6() {}

  virtual bool IsConnected() OVERRIDE { return true; }

  virtual bool Connect(const GURL& url) OVERRIDE { return true; }

  virtual bool Send(const std::string& message) OVERRIDE { return true; }

  virtual SyncWebSocket::StatusCode ReceiveNextMessage(
      std::string* message,
      const base::TimeDelta& timeout) OVERRIDE {
    if (messages_->empty())
      return SyncWebSocket::kDisconnected;
    *message = messages_->front();
    messages_->pop_front();
    return SyncWebSocket::kOk;
  }

  virtual bool HasNextMessage() OVERRIDE { return messages_->size(); }

 private:
  std::list<std::string>* messages_;
};

class MockDevToolsEventListener : public DevToolsEventListener {
 public:
  MockDevToolsEventListener() : id_(1) {}
  virtual ~MockDevToolsEventListener() {}

  virtual Status OnConnected(DevToolsClient* client) OVERRIDE {
    return Status(kOk);
  }

  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE {
    id_++;
    Status status = client->SendCommand("hello", params);
    id_--;
    if (id_ == 3) {
      EXPECT_EQ(kUnexpectedAlertOpen, status.code());
    } else {
      EXPECT_EQ(kOk, status.code());
    }
    return Status(kOk);
  }

 private:
  int id_;
};

scoped_ptr<SyncWebSocket> CreateMockSyncWebSocket6(
    std::list<std::string>* messages) {
  return scoped_ptr<MockSyncWebSocket6>(new MockSyncWebSocket6(messages))
      .PassAs<SyncWebSocket>();
}

}  // namespace

TEST_F(DevToolsClientImplTest, BlockedByAlert) {
  std::list<std::string> msgs;
  SyncWebSocketFactory factory = base::Bind(&CreateMockSyncWebSocket6, &msgs);
  DevToolsClientImpl client(
      factory, "http://url", "id", base::Bind(&CloserFunc));
  msgs.push_back(
      "{\"method\": \"Page.javascriptDialogOpening\", \"params\": {}}");
  msgs.push_back("{\"id\": 2, \"result\": {}}");
  base::DictionaryValue params;
  ASSERT_EQ(kUnexpectedAlertOpen,
            client.SendCommand("first", params).code());
}

TEST_F(DevToolsClientImplTest, CorrectlyDeterminesWhichIsBlockedByAlert) {
  // OUT                 | IN
  //                       FirstEvent
  // hello (id=1)
  //                       SecondEvent
  // hello (id=2)
  //                       ThirdEvent
  // hello (id=3)
  //                       FourthEvent
  // hello (id=4)
  //                       response for 1
  //                       alert
  // hello (id=5)
  // round trip command (id=6)
  //                       response for 2
  //                       response for 4
  //                       response for 5
  //                       response for 6
  std::list<std::string> msgs;
  SyncWebSocketFactory factory = base::Bind(&CreateMockSyncWebSocket6, &msgs);
  DevToolsClientImpl client(
      factory, "http://url", "id", base::Bind(&CloserFunc));
  MockDevToolsEventListener listener;
  client.AddListener(&listener);
  msgs.push_back("{\"method\": \"FirstEvent\", \"params\": {}}");
  msgs.push_back("{\"method\": \"SecondEvent\", \"params\": {}}");
  msgs.push_back("{\"method\": \"ThirdEvent\", \"params\": {}}");
  msgs.push_back("{\"method\": \"FourthEvent\", \"params\": {}}");
  msgs.push_back("{\"id\": 1, \"result\": {}}");
  msgs.push_back(
      "{\"method\": \"Page.javascriptDialogOpening\", \"params\": {}}");
  msgs.push_back("{\"id\": 2, \"result\": {}}");
  msgs.push_back("{\"id\": 4, \"result\": {}}");
  msgs.push_back("{\"id\": 5, \"result\": {}}");
  msgs.push_back("{\"id\": 6, \"result\": {}}");
  ASSERT_EQ(kOk, client.HandleReceivedEvents().code());
}

namespace {

class MockCommandListener : public DevToolsEventListener {
 public:
  MockCommandListener() {}
  virtual ~MockCommandListener() {}

  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE {
    msgs_.push_back(method);
    return Status(kOk);
  }

  virtual Status OnCommandSuccess(DevToolsClient* client,
                                  const std::string& method) OVERRIDE {
    msgs_.push_back(method);
    if (!callback_.is_null())
      callback_.Run(client);
    return Status(kOk);
  }

  base::Callback<void(DevToolsClient*)> callback_;
  std::list<std::string> msgs_;
};

void HandleReceivedEvents(DevToolsClient* client) {
  EXPECT_EQ(kOk, client->HandleReceivedEvents().code());
}

}  // namespace

TEST_F(DevToolsClientImplTest, ReceivesCommandResponse) {
  std::list<std::string> msgs;
  SyncWebSocketFactory factory = base::Bind(&CreateMockSyncWebSocket6, &msgs);
  DevToolsClientImpl client(
      factory, "http://url", "id", base::Bind(&CloserFunc));
  MockCommandListener listener1;
  listener1.callback_ = base::Bind(&HandleReceivedEvents);
  MockCommandListener listener2;
  client.AddListener(&listener1);
  client.AddListener(&listener2);
  msgs.push_back("{\"id\": 1, \"result\": {}}");
  msgs.push_back("{\"method\": \"event\", \"params\": {}}");
  base::DictionaryValue params;
  ASSERT_EQ(kOk, client.SendCommand("cmd", params).code());
  ASSERT_EQ(2u, listener2.msgs_.size());
  ASSERT_EQ("cmd", listener2.msgs_.front());
  ASSERT_EQ("event", listener2.msgs_.back());
}
