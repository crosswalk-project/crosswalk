// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/devtools_client_impl.h"

#include "base/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/devtools_event_listener.h"
#include "chrome/test/chromedriver/chrome/log.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/util.h"
#include "chrome/test/chromedriver/net/sync_websocket.h"
#include "chrome/test/chromedriver/net/url_request_context_getter.h"

namespace {

const char* kInspectorContextError =
    "Execution context with given id not found.";

Status ParseInspectorError(const std::string& error_json) {
  scoped_ptr<base::Value> error(base::JSONReader::Read(error_json));
  base::DictionaryValue* error_dict;
  if (!error || !error->GetAsDictionary(&error_dict))
    return Status(kUnknownError, "inspector error with no error message");
  std::string error_message;
  if (error_dict->GetString("message", &error_message) &&
      error_message == kInspectorContextError) {
    return Status(kNoSuchExecutionContext);
  }
  return Status(kUnknownError, "unhandled inspector error: " + error_json);
}

class ScopedIncrementer {
 public:
  explicit ScopedIncrementer(int* count) : count_(count) {
    (*count_)++;
  }
  ~ScopedIncrementer() {
    (*count_)--;
  }

 private:
  int* count_;
};

Status ConditionIsMet(bool* is_condition_met) {
  *is_condition_met = true;
  return Status(kOk);
}

}  // namespace

namespace internal {

InspectorEvent::InspectorEvent() {}

InspectorEvent::~InspectorEvent() {}

InspectorCommandResponse::InspectorCommandResponse() {}

InspectorCommandResponse::~InspectorCommandResponse() {}

}  // namespace internal

DevToolsClientImpl::DevToolsClientImpl(
    const SyncWebSocketFactory& factory,
    const std::string& url,
    const std::string& id,
    const FrontendCloserFunc& frontend_closer_func)
    : socket_(factory.Run().Pass()),
      url_(url),
      crashed_(false),
      id_(id),
      frontend_closer_func_(frontend_closer_func),
      parser_func_(base::Bind(&internal::ParseInspectorMessage)),
      unnotified_event_(NULL),
      next_id_(1),
      stack_count_(0) {}

DevToolsClientImpl::DevToolsClientImpl(
    const SyncWebSocketFactory& factory,
    const std::string& url,
    const std::string& id,
    const FrontendCloserFunc& frontend_closer_func,
    const ParserFunc& parser_func)
    : socket_(factory.Run().Pass()),
      url_(url),
      crashed_(false),
      id_(id),
      frontend_closer_func_(frontend_closer_func),
      parser_func_(parser_func),
      unnotified_event_(NULL),
      next_id_(1),
      stack_count_(0) {}

DevToolsClientImpl::~DevToolsClientImpl() {}

void DevToolsClientImpl::SetParserFuncForTesting(
    const ParserFunc& parser_func) {
  parser_func_ = parser_func;
}

const std::string& DevToolsClientImpl::GetId() {
  return id_;
}

bool DevToolsClientImpl::WasCrashed() {
  return crashed_;
}

Status DevToolsClientImpl::ConnectIfNecessary() {
  if (stack_count_)
    return Status(kUnknownError, "cannot connect when nested");

  if (socket_->IsConnected())
    return Status(kOk);

  if (!socket_->Connect(url_)) {
    // Try to close devtools frontend and then reconnect.
    Status status = frontend_closer_func_.Run();
    if (status.IsError())
      return status;
    if (!socket_->Connect(url_))
      return Status(kDisconnected, "unable to connect to renderer");
  }

  unnotified_connect_listeners_ = listeners_;
  unnotified_event_listeners_.clear();
  response_info_map_.clear();

  // Notify all listeners of the new connection. Do this now so that any errors
  // that occur are reported now instead of later during some unrelated call.
  // Also gives listeners a chance to send commands before other clients.
  return EnsureListenersNotifiedOfConnect();
}

Status DevToolsClientImpl::SendCommand(
    const std::string& method,
    const base::DictionaryValue& params) {
  scoped_ptr<base::DictionaryValue> result;
  return SendCommandInternal(method, params, &result);
}

Status DevToolsClientImpl::SendCommandAndGetResult(
    const std::string& method,
    const base::DictionaryValue& params,
    scoped_ptr<base::DictionaryValue>* result) {
  scoped_ptr<base::DictionaryValue> intermediate_result;
  Status status = SendCommandInternal(method, params, &intermediate_result);
  if (status.IsError())
    return status;
  if (!intermediate_result)
    return Status(kUnknownError, "inspector response missing result");
  result->reset(intermediate_result.release());
  return Status(kOk);
}

void DevToolsClientImpl::AddListener(DevToolsEventListener* listener) {
  CHECK(listener);
  listeners_.push_back(listener);
}

Status DevToolsClientImpl::HandleReceivedEvents() {
  return HandleEventsUntil(base::Bind(&ConditionIsMet), base::TimeDelta());
}

Status DevToolsClientImpl::HandleEventsUntil(
    const ConditionalFunc& conditional_func, const base::TimeDelta& timeout) {
  if (!socket_->IsConnected())
    return Status(kDisconnected, "not connected to DevTools");

  base::TimeTicks deadline = base::TimeTicks::Now() + timeout;
  base::TimeDelta next_message_timeout = timeout;
  while (true) {
    if (!socket_->HasNextMessage()) {
      bool is_condition_met = false;
      Status status = conditional_func.Run(&is_condition_met);
      if (status.IsError())
        return status;
      if (is_condition_met)
        return Status(kOk);
    }

    Status status = ProcessNextMessage(-1, next_message_timeout);
    if (status.IsError())
      return status;
    next_message_timeout = deadline - base::TimeTicks::Now();
  }
}

DevToolsClientImpl::ResponseInfo::ResponseInfo(const std::string& method)
    : state(kWaiting), method(method) {}

DevToolsClientImpl::ResponseInfo::~ResponseInfo() {}

Status DevToolsClientImpl::SendCommandInternal(
    const std::string& method,
    const base::DictionaryValue& params,
    scoped_ptr<base::DictionaryValue>* result) {
  if (!socket_->IsConnected())
    return Status(kDisconnected, "not connected to DevTools");

  int command_id = next_id_++;
  base::DictionaryValue command;
  command.SetInteger("id", command_id);
  command.SetString("method", method);
  command.Set("params", params.DeepCopy());
  std::string message = SerializeValue(&command);
  if (IsVLogOn(1)) {
    VLOG(1) << "DEVTOOLS COMMAND " << method << " (id=" << command_id << ") "
            << FormatValueForDisplay(params);
  }
  if (!socket_->Send(message))
    return Status(kDisconnected, "unable to send message to renderer");

  linked_ptr<ResponseInfo> response_info =
      make_linked_ptr(new ResponseInfo(method));
  response_info_map_[command_id] = response_info;
  while (response_info->state == kWaiting) {
    Status status = ProcessNextMessage(
        command_id, base::TimeDelta::FromMinutes(10));
    if (status.IsError()) {
      if (response_info->state == kReceived)
        response_info_map_.erase(command_id);
      return status;
    }
  }
  if (response_info->state == kBlocked) {
    response_info->state = kIgnored;
    return Status(kUnexpectedAlertOpen);
  }
  CHECK_EQ(response_info->state, kReceived);
  internal::InspectorCommandResponse& response = response_info->response;
  if (!response.result)
    return ParseInspectorError(response.error);
  *result = response.result.Pass();
  return Status(kOk);
}

Status DevToolsClientImpl::ProcessNextMessage(
    int expected_id,
    const base::TimeDelta& timeout) {
  ScopedIncrementer increment_stack_count(&stack_count_);

  Status status = EnsureListenersNotifiedOfConnect();
  if (status.IsError())
    return status;
  status = EnsureListenersNotifiedOfEvent();
  if (status.IsError())
    return status;
  status = EnsureListenersNotifiedOfCommandResponse();
  if (status.IsError())
    return status;

  // The command response may have already been received or blocked while
  // notifying listeners.
  if (expected_id != -1 && response_info_map_[expected_id]->state != kWaiting)
    return Status(kOk);

  if (crashed_)
    return Status(kTabCrashed);

  std::string message;
  switch (socket_->ReceiveNextMessage(&message, timeout)) {
    case SyncWebSocket::kOk:
      break;
    case SyncWebSocket::kDisconnected: {
      std::string err = "Unable to receive message from renderer";
      LOG(ERROR) << err;
      return Status(kDisconnected, err);
    }
    case SyncWebSocket::kTimeout: {
      std::string err =
          "Timed out receiving message from renderer: " +
          base::StringPrintf("%.3lf", timeout.InSecondsF());
      LOG(ERROR) << err;
      return Status(kTimeout, err);
    }
    default:
      NOTREACHED();
      break;
  }

  internal::InspectorMessageType type;
  internal::InspectorEvent event;
  internal::InspectorCommandResponse response;
  if (!parser_func_.Run(message, expected_id, &type, &event, &response)) {
    LOG(ERROR) << "Bad inspector message: " << message;
    return Status(kUnknownError, "bad inspector message: " + message);
  }

  if (type == internal::kEventMessageType)
    return ProcessEvent(event);
  CHECK_EQ(type, internal::kCommandResponseMessageType);
  return ProcessCommandResponse(response);
}

Status DevToolsClientImpl::ProcessEvent(const internal::InspectorEvent& event) {
  if (IsVLogOn(1)) {
    VLOG(1) << "DEVTOOLS EVENT " << event.method << " "
            << FormatValueForDisplay(*event.params);
  }
  unnotified_event_listeners_ = listeners_;
  unnotified_event_ = &event;
  Status status = EnsureListenersNotifiedOfEvent();
  unnotified_event_ = NULL;
  if (status.IsError())
    return status;
  if (event.method == "Inspector.detached")
    return Status(kDisconnected, "received Inspector.detached event");
  if (event.method == "Inspector.targetCrashed") {
    crashed_ = true;
    return Status(kTabCrashed);
  }
  if (event.method == "Page.javascriptDialogOpening") {
    // A command may have opened the dialog, which will block the response.
    // To find out which one (if any), do a round trip with a simple command
    // to the renderer and afterwards see if any of the commands still haven't
    // received a response.
    // This relies on the fact that DevTools commands are processed
    // sequentially. This may break if any of the commands are asynchronous.
    // If for some reason the round trip command fails, mark all the waiting
    // commands as blocked and return the error. This is better than risking
    // a hang.
    int max_id = next_id_;
    base::DictionaryValue enable_params;
    enable_params.SetString("purpose", "detect if alert blocked any cmds");
    Status enable_status = SendCommand("Inspector.enable", enable_params);
    for (ResponseInfoMap::const_iterator iter = response_info_map_.begin();
         iter != response_info_map_.end(); ++iter) {
      if (iter->first > max_id)
        continue;
      if (iter->second->state == kWaiting)
        iter->second->state = kBlocked;
    }
    if (enable_status.IsError())
      return status;
  }
  return Status(kOk);
}

Status DevToolsClientImpl::ProcessCommandResponse(
    const internal::InspectorCommandResponse& response) {
  ResponseInfoMap::iterator iter = response_info_map_.find(response.id);
  if (IsVLogOn(1)) {
    std::string method, result;
    if (iter != response_info_map_.end())
      method = iter->second->method;
    if (response.result)
      result = FormatValueForDisplay(*response.result);
    else
      result = response.error;
    VLOG(1) << "DEVTOOLS RESPONSE " << method << " (id=" << response.id
            << ") " << result;
  }

  if (iter == response_info_map_.end())
    return Status(kUnknownError, "unexpected command response");

  linked_ptr<ResponseInfo> response_info = response_info_map_[response.id];
  if (response_info->state == kReceived)
    return Status(kUnknownError, "received multiple command responses");

  if (response_info->state == kIgnored) {
    response_info_map_.erase(response.id);
  } else {
    response_info->state = kReceived;
    response_info->response.id = response.id;
    response_info->response.error = response.error;
    if (response.result)
      response_info->response.result.reset(response.result->DeepCopy());
  }

  if (response.result) {
    unnotified_cmd_response_listeners_ = listeners_;
    unnotified_cmd_response_info_ = response_info;
    Status status = EnsureListenersNotifiedOfCommandResponse();
    unnotified_cmd_response_info_.reset();
    if (status.IsError())
      return status;
  }
  return Status(kOk);
}

Status DevToolsClientImpl::EnsureListenersNotifiedOfConnect() {
  while (unnotified_connect_listeners_.size()) {
    DevToolsEventListener* listener = unnotified_connect_listeners_.front();
    unnotified_connect_listeners_.pop_front();
    Status status = listener->OnConnected(this);
    if (status.IsError())
      return status;
  }
  return Status(kOk);
}

Status DevToolsClientImpl::EnsureListenersNotifiedOfEvent() {
  while (unnotified_event_listeners_.size()) {
    DevToolsEventListener* listener = unnotified_event_listeners_.front();
    unnotified_event_listeners_.pop_front();
    Status status = listener->OnEvent(
        this, unnotified_event_->method, *unnotified_event_->params);
    if (status.IsError())
      return status;
  }
  return Status(kOk);
}

Status DevToolsClientImpl::EnsureListenersNotifiedOfCommandResponse() {
  while (unnotified_cmd_response_listeners_.size()) {
    DevToolsEventListener* listener =
        unnotified_cmd_response_listeners_.front();
    unnotified_cmd_response_listeners_.pop_front();
    Status status =
        listener->OnCommandSuccess(this, unnotified_cmd_response_info_->method);
    if (status.IsError())
      return status;
  }
  return Status(kOk);
}

namespace internal {

bool ParseInspectorMessage(
    const std::string& message,
    int expected_id,
    InspectorMessageType* type,
    InspectorEvent* event,
    InspectorCommandResponse* command_response) {
  scoped_ptr<base::Value> message_value(base::JSONReader::Read(message));
  base::DictionaryValue* message_dict;
  if (!message_value || !message_value->GetAsDictionary(&message_dict))
    return false;

  int id;
  if (!message_dict->HasKey("id")) {
    std::string method;
    if (!message_dict->GetString("method", &method))
      return false;
    base::DictionaryValue* params = NULL;
    message_dict->GetDictionary("params", &params);

    *type = kEventMessageType;
    event->method = method;
    if (params)
      event->params.reset(params->DeepCopy());
    else
      event->params.reset(new base::DictionaryValue());
    return true;
  } else if (message_dict->GetInteger("id", &id)) {
    base::DictionaryValue* unscoped_error = NULL;
    base::DictionaryValue* unscoped_result = NULL;
    if (!message_dict->GetDictionary("error", &unscoped_error) &&
        !message_dict->GetDictionary("result", &unscoped_result))
      return false;

    *type = kCommandResponseMessageType;
    command_response->id = id;
    if (unscoped_result)
      command_response->result.reset(unscoped_result->DeepCopy());
    else
      base::JSONWriter::Write(unscoped_error, &command_response->error);
    return true;
  }
  return false;
}

}  // namespace internal
