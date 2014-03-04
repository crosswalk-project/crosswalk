// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_DEVTOOLS_CLIENT_IMPL_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_DEVTOOLS_CLIENT_IMPL_H_

#include <list>
#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/test/chromedriver/chrome/devtools_client.h"
#include "chrome/test/chromedriver/net/sync_websocket_factory.h"
#include "url/gurl.h"

namespace base {
class DictionaryValue;
}

namespace internal {

enum InspectorMessageType {
  kEventMessageType = 0,
  kCommandResponseMessageType
};

struct InspectorEvent {
  InspectorEvent();
  ~InspectorEvent();
  std::string method;
  scoped_ptr<base::DictionaryValue> params;
};

struct InspectorCommandResponse {
  InspectorCommandResponse();
  ~InspectorCommandResponse();
  int id;
  std::string error;
  scoped_ptr<base::DictionaryValue> result;
};

}  // namespace internal

class DevToolsEventListener;
class Status;
class SyncWebSocket;

class DevToolsClientImpl : public DevToolsClient {
 public:
  typedef base::Callback<Status()> FrontendCloserFunc;
  DevToolsClientImpl(const SyncWebSocketFactory& factory,
                     const std::string& url,
                     const std::string& id,
                     const FrontendCloserFunc& frontend_closer_func);

  typedef base::Callback<bool(
      const std::string&,
      int,
      internal::InspectorMessageType*,
      internal::InspectorEvent*,
      internal::InspectorCommandResponse*)> ParserFunc;
  DevToolsClientImpl(const SyncWebSocketFactory& factory,
                     const std::string& url,
                     const std::string& id,
                     const FrontendCloserFunc& frontend_closer_func,
                     const ParserFunc& parser_func);

  virtual ~DevToolsClientImpl();

  void SetParserFuncForTesting(const ParserFunc& parser_func);

  // Overridden from DevToolsClient:
  virtual const std::string& GetId() OVERRIDE;
  virtual bool WasCrashed() OVERRIDE;
  virtual Status ConnectIfNecessary() OVERRIDE;
  virtual Status SendCommand(const std::string& method,
                             const base::DictionaryValue& params) OVERRIDE;
  virtual Status SendCommandAndGetResult(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result) OVERRIDE;
  virtual void AddListener(DevToolsEventListener* listener) OVERRIDE;
  virtual Status HandleEventsUntil(
      const ConditionalFunc& conditional_func,
      const base::TimeDelta& timeout) OVERRIDE;
  virtual Status HandleReceivedEvents() OVERRIDE;

 private:
  enum ResponseState {
    // The client is waiting for the response.
    kWaiting,
    // The command response will not be received because it is blocked by an
    // alert that the command triggered.
    kBlocked,
    // The client no longer cares about the response.
    kIgnored,
    // The response has been received.
    kReceived
  };
  struct ResponseInfo {
    explicit ResponseInfo(const std::string& method);
    ~ResponseInfo();

    ResponseState state;
    std::string method;
    internal::InspectorCommandResponse response;
  };
  typedef std::map<int, linked_ptr<ResponseInfo> > ResponseInfoMap;

  Status SendCommandInternal(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result);
  Status ProcessNextMessage(int expected_id, const base::TimeDelta& timeout);
  Status ProcessEvent(const internal::InspectorEvent& event);
  Status ProcessCommandResponse(
      const internal::InspectorCommandResponse& response);
  Status EnsureListenersNotifiedOfConnect();
  Status EnsureListenersNotifiedOfEvent();
  Status EnsureListenersNotifiedOfCommandResponse();

  scoped_ptr<SyncWebSocket> socket_;
  GURL url_;
  bool crashed_;
  const std::string id_;
  FrontendCloserFunc frontend_closer_func_;
  ParserFunc parser_func_;
  std::list<DevToolsEventListener*> listeners_;
  std::list<DevToolsEventListener*> unnotified_connect_listeners_;
  std::list<DevToolsEventListener*> unnotified_event_listeners_;
  const internal::InspectorEvent* unnotified_event_;
  std::list<DevToolsEventListener*> unnotified_cmd_response_listeners_;
  linked_ptr<ResponseInfo> unnotified_cmd_response_info_;
  ResponseInfoMap response_info_map_;
  int next_id_;
  int stack_count_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsClientImpl);
};

namespace internal {

bool ParseInspectorMessage(
    const std::string& message,
    int expected_id,
    InspectorMessageType* type,
    InspectorEvent* event,
    InspectorCommandResponse* command_response);

}  // namespace internal

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_DEVTOOLS_CLIENT_IMPL_H_
