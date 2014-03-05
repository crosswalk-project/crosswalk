// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/chrome/web_view_impl.h"

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/debugger_tracker.h"
#include "xwalk/test/chromedriver/chrome/devtools_client_impl.h"
#include "xwalk/test/chromedriver/chrome/dom_tracker.h"
#include "xwalk/test/chromedriver/chrome/frame_tracker.h"
#include "xwalk/test/chromedriver/chrome/geolocation_override_manager.h"
#include "xwalk/test/chromedriver/chrome/heap_snapshot_taker.h"
#include "xwalk/test/chromedriver/chrome/javascript_dialog_manager.h"
#include "xwalk/test/chromedriver/chrome/js.h"
#include "xwalk/test/chromedriver/chrome/navigation_tracker.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/ui_events.h"

namespace {

Status GetContextIdForFrame(FrameTracker* tracker,
                            const std::string& frame,
                            int* context_id) {
  if (frame.empty()) {
    *context_id = 0;
    return Status(kOk);
  }
  Status status = tracker->GetContextIdForFrame(frame, context_id);
  if (status.IsError())
    return status;
  return Status(kOk);
}

const char* GetAsString(MouseEventType type) {
  switch (type) {
    case kPressedMouseEventType:
      return "mousePressed";
    case kReleasedMouseEventType:
      return "mouseReleased";
    case kMovedMouseEventType:
      return "mouseMoved";
    default:
      return "";
  }
}

const char* GetAsString(TouchEventType type) {
  switch (type) {
    case kTouchStart:
      return "touchStart";
    case kTouchEnd:
      return "touchEnd";
    case kTouchMove:
      return "touchMove";
    default:
      return "";
  }
}

const char* GetPointStateString(TouchEventType type) {
  switch (type) {
    case kTouchStart:
      return "touchPressed";
    case kTouchEnd:
      return "touchReleased";
    case kTouchMove:
      return "touchMoved";
    default:
      return "";
  }
}

const char* GetAsString(MouseButton button) {
  switch (button) {
    case kLeftMouseButton:
      return "left";
    case kMiddleMouseButton:
      return "middle";
    case kRightMouseButton:
      return "right";
    case kNoneMouseButton:
      return "none";
    default:
      return "";
  }
}

const char* GetAsString(KeyEventType type) {
  switch (type) {
    case kKeyDownEventType:
      return "keyDown";
    case kKeyUpEventType:
      return "keyUp";
    case kRawKeyDownEventType:
      return "rawKeyDown";
    case kCharEventType:
      return "char";
    default:
      return "";
  }
}

}  // namespace

WebViewImpl::WebViewImpl(const std::string& id,
                         int build_no,
                         scoped_ptr<DevToolsClient> client)
    : id_(id),
      build_no_(build_no),
      dom_tracker_(new DomTracker(client.get())),
      frame_tracker_(new FrameTracker(client.get())),
      navigation_tracker_(new NavigationTracker(client.get())),
      dialog_manager_(new JavaScriptDialogManager(client.get())),
      geolocation_override_manager_(
          new GeolocationOverrideManager(client.get())),
      heap_snapshot_taker_(new HeapSnapshotTaker(client.get())),
      debugger_(new DebuggerTracker(client.get())),
      client_(client.release()) {}

WebViewImpl::~WebViewImpl() {}

std::string WebViewImpl::GetId() {
  return id_;
}

bool WebViewImpl::WasCrashed() {
  return client_->WasCrashed();
}

Status WebViewImpl::ConnectIfNecessary() {
  return client_->ConnectIfNecessary();
}

Status WebViewImpl::HandleReceivedEvents() {
  return client_->HandleReceivedEvents();
}

Status WebViewImpl::Load(const std::string& url) {
  // Javascript URLs will cause a hang while waiting for the page to stop
  // loading, so just disallow.
  if (StartsWithASCII(url, "javascript:", false))
    return Status(kUnknownError, "unsupported protocol");
  base::DictionaryValue params;
  params.SetString("url", url);
  return client_->SendCommand("Page.navigate", params);
}

Status WebViewImpl::Reload() {
  base::DictionaryValue params;
  params.SetBoolean("ignoreCache", false);
  return client_->SendCommand("Page.reload", params);
}

Status WebViewImpl::EvaluateScript(const std::string& frame,
                                   const std::string& expression,
                                   scoped_ptr<base::Value>* result) {
  int context_id;
  Status status = GetContextIdForFrame(frame_tracker_.get(), frame,
                                       &context_id);
  if (status.IsError())
    return status;
  return internal::EvaluateScriptAndGetValue(
      client_.get(), context_id, expression, result);
}

Status WebViewImpl::CallFunction(const std::string& frame,
                                 const std::string& function,
                                 const base::ListValue& args,
                                 scoped_ptr<base::Value>* result) {
  std::string json;
  base::JSONWriter::Write(&args, &json);
  // TODO(zachconrad): Second null should be array of shadow host ids.
  std::string expression = base::StringPrintf(
      "(%s).apply(null, [null, %s, %s])",
      kCallFunctionScript,
      function.c_str(),
      json.c_str());
  scoped_ptr<base::Value> temp_result;
  Status status = EvaluateScript(frame, expression, &temp_result);
  if (status.IsError())
    return status;

  return internal::ParseCallFunctionResult(*temp_result, result);
}

Status WebViewImpl::CallAsyncFunction(const std::string& frame,
                                      const std::string& function,
                                      const base::ListValue& args,
                                      const base::TimeDelta& timeout,
                                      scoped_ptr<base::Value>* result) {
  return CallAsyncFunctionInternal(
      frame, function, args, false, timeout, result);
}

Status WebViewImpl::CallUserAsyncFunction(const std::string& frame,
                                          const std::string& function,
                                          const base::ListValue& args,
                                          const base::TimeDelta& timeout,
                                          scoped_ptr<base::Value>* result) {
  return CallAsyncFunctionInternal(
      frame, function, args, true, timeout, result);
}

Status WebViewImpl::GetFrameByFunction(const std::string& frame,
                                       const std::string& function,
                                       const base::ListValue& args,
                                       std::string* out_frame) {
  int context_id;
  Status status = GetContextIdForFrame(frame_tracker_.get(), frame,
                                       &context_id);
  if (status.IsError())
    return status;
  bool found_node;
  int node_id;
  status = internal::GetNodeIdFromFunction(
      client_.get(), context_id, function, args, &found_node, &node_id);
  if (status.IsError())
    return status;
  if (!found_node)
    return Status(kNoSuchFrame);
  return dom_tracker_->GetFrameIdForNode(node_id, out_frame);
}

Status WebViewImpl::DispatchMouseEvents(const std::list<MouseEvent>& events,
                                        const std::string& frame) {
  for (std::list<MouseEvent>::const_iterator it = events.begin();
       it != events.end(); ++it) {
    base::DictionaryValue params;
    params.SetString("type", GetAsString(it->type));
    params.SetInteger("x", it->x);
    params.SetInteger("y", it->y);
    params.SetInteger("modifiers", it->modifiers);
    params.SetString("button", GetAsString(it->button));
    params.SetInteger("clickCount", it->click_count);
    Status status = client_->SendCommand("Input.dispatchMouseEvent", params);
    if (status.IsError())
      return status;
    if (build_no_ < 1569 && it->button == kRightMouseButton &&
        it->type == kReleasedMouseEventType) {
      base::ListValue args;
      args.AppendInteger(it->x);
      args.AppendInteger(it->y);
      args.AppendInteger(it->modifiers);
      scoped_ptr<base::Value> result;
      status = CallFunction(
          frame, kDispatchContextMenuEventScript, args, &result);
      if (status.IsError())
        return status;
    }
  }
  return Status(kOk);
}

Status WebViewImpl::DispatchTouchEvents(const std::list<TouchEvent>& events) {
  for (std::list<TouchEvent>::const_iterator it = events.begin();
       it != events.end(); ++it) {
    base::DictionaryValue params;
    params.SetString("type", GetAsString(it->type));
    scoped_ptr<base::ListValue> point_list(new base::ListValue);
    scoped_ptr<base::DictionaryValue> point(new base::DictionaryValue);
    point->SetString("state", GetPointStateString(it->type));
    point->SetInteger("x", it->x);
    point->SetInteger("y", it->y);
    point_list->Set(0, point.release());
    params.Set("touchPoints", point_list.release());
    Status status = client_->SendCommand("Input.dispatchTouchEvent", params);
    if (status.IsError())
      return status;
  }
  return Status(kOk);
}

Status WebViewImpl::DispatchKeyEvents(const std::list<KeyEvent>& events) {
  for (std::list<KeyEvent>::const_iterator it = events.begin();
       it != events.end(); ++it) {
    base::DictionaryValue params;
    params.SetString("type", GetAsString(it->type));
    if (it->modifiers & kNumLockKeyModifierMask) {
      params.SetBoolean("isKeypad", true);
      params.SetInteger("modifiers",
                        it->modifiers & ~kNumLockKeyModifierMask);
    } else {
      params.SetInteger("modifiers", it->modifiers);
    }
    params.SetString("text", it->modified_text);
    params.SetString("unmodifiedText", it->unmodified_text);
    params.SetInteger("nativeVirtualKeyCode", it->key_code);
    params.SetInteger("windowsVirtualKeyCode", it->key_code);
    Status status = client_->SendCommand("Input.dispatchKeyEvent", params);
    if (status.IsError())
      return status;
  }
  return Status(kOk);
}

Status WebViewImpl::GetCookies(scoped_ptr<base::ListValue>* cookies) {
  base::DictionaryValue params;
  scoped_ptr<base::DictionaryValue> result;
  Status status = client_->SendCommandAndGetResult(
      "Page.getCookies", params, &result);
  if (status.IsError())
    return status;
  base::ListValue* cookies_tmp;
  if (!result->GetList("cookies", &cookies_tmp))
    return Status(kUnknownError, "DevTools didn't return cookies");
  cookies->reset(cookies_tmp->DeepCopy());
  return Status(kOk);
}

Status WebViewImpl::DeleteCookie(const std::string& name,
                                 const std::string& url) {
  base::DictionaryValue params;
  params.SetString("cookieName", name);
  params.SetString("url", url);
  return client_->SendCommand("Page.deleteCookie", params);
}

Status WebViewImpl::WaitForPendingNavigations(const std::string& frame_id,
                                              const base::TimeDelta& timeout,
                                              bool stop_load_on_timeout) {
  VLOG(0) << "Waiting for pending navigations...";
  Status status = client_->HandleEventsUntil(
      base::Bind(&WebViewImpl::IsNotPendingNavigation,
                 base::Unretained(this),
                 frame_id),
      timeout);
  if (status.code() == kTimeout && stop_load_on_timeout) {
    VLOG(0) << "Timed out. Stopping navigation...";
    scoped_ptr<base::Value> unused_value;
    EvaluateScript(std::string(), "window.stop();", &unused_value);
    Status new_status = client_->HandleEventsUntil(
        base::Bind(&WebViewImpl::IsNotPendingNavigation, base::Unretained(this),
                   frame_id),
        base::TimeDelta::FromSeconds(10));
    if (new_status.IsError())
      status = new_status;
  }
  VLOG(0) << "Done waiting for pending navigations";
  return status;
}

Status WebViewImpl::IsPendingNavigation(const std::string& frame_id,
                                        bool* is_pending) {
  return navigation_tracker_->IsPendingNavigation(frame_id, is_pending);
}

JavaScriptDialogManager* WebViewImpl::GetJavaScriptDialogManager() {
  return dialog_manager_.get();
}

Status WebViewImpl::OverrideGeolocation(const Geoposition& geoposition) {
  return geolocation_override_manager_->OverrideGeolocation(geoposition);
}

Status WebViewImpl::CaptureScreenshot(std::string* screenshot) {
  base::DictionaryValue params;
  scoped_ptr<base::DictionaryValue> result;
  Status status = client_->SendCommandAndGetResult(
      "Page.captureScreenshot", params, &result);
  if (status.IsError())
    return status;
  if (!result->GetString("data", screenshot))
    return Status(kUnknownError, "expected string 'data' in response");
  return Status(kOk);
}

Status WebViewImpl::SetFileInputFiles(
    const std::string& frame,
    const base::DictionaryValue& element,
    const std::vector<base::FilePath>& files) {
  base::ListValue file_list;
  for (size_t i = 0; i < files.size(); ++i) {
    if (!files[i].IsAbsolute()) {
      return Status(kUnknownError,
                    "path is not absolute: " + files[i].AsUTF8Unsafe());
    }
    if (files[i].ReferencesParent()) {
      return Status(kUnknownError,
                    "path is not canonical: " + files[i].AsUTF8Unsafe());
    }
    file_list.AppendString(files[i].value());
  }

  int context_id;
  Status status = GetContextIdForFrame(frame_tracker_.get(), frame,
                                       &context_id);
  if (status.IsError())
    return status;
  base::ListValue args;
  args.Append(element.DeepCopy());
  bool found_node;
  int node_id;
  status = internal::GetNodeIdFromFunction(
      client_.get(), context_id, "function(element) { return element; }",
      args, &found_node, &node_id);
  if (status.IsError())
    return status;
  if (!found_node)
    return Status(kUnknownError, "no node ID for file input");
  base::DictionaryValue params;
  params.SetInteger("nodeId", node_id);
  params.Set("files", file_list.DeepCopy());
  return client_->SendCommand("DOM.setFileInputFiles", params);
}

Status WebViewImpl::TakeHeapSnapshot(scoped_ptr<base::Value>* snapshot) {
  return heap_snapshot_taker_->TakeSnapshot(snapshot);
}

Status WebViewImpl::CallAsyncFunctionInternal(const std::string& frame,
                                              const std::string& function,
                                              const base::ListValue& args,
                                              bool is_user_supplied,
                                              const base::TimeDelta& timeout,
                                              scoped_ptr<base::Value>* result) {
  base::ListValue async_args;
  async_args.AppendString("return (" + function + ").apply(null, arguments);");
  async_args.Append(args.DeepCopy());
  async_args.AppendBoolean(is_user_supplied);
  async_args.AppendInteger(timeout.InMilliseconds());
  scoped_ptr<base::Value> tmp;
  Status status = CallFunction(
      frame, kExecuteAsyncScriptScript, async_args, &tmp);
  if (status.IsError())
    return status;

  const char* kDocUnloadError = "document unloaded while waiting for result";
  std::string kQueryResult = base::StringPrintf(
      "function() {"
      "  var info = document.$chrome_asyncScriptInfo;"
      "  if (!info)"
      "    return {status: %d, value: '%s'};"
      "  var result = info.result;"
      "  if (!result)"
      "    return {status: 0};"
      "  delete info.result;"
      "  return result;"
      "}",
      kJavaScriptError,
      kDocUnloadError);

  while (true) {
    base::ListValue no_args;
    scoped_ptr<base::Value> query_value;
    Status status = CallFunction(frame, kQueryResult, no_args, &query_value);
    if (status.IsError()) {
      if (status.code() == kNoSuchFrame)
        return Status(kJavaScriptError, kDocUnloadError);
      return status;
    }

    base::DictionaryValue* result_info = NULL;
    if (!query_value->GetAsDictionary(&result_info))
      return Status(kUnknownError, "async result info is not a dictionary");
    int status_code;
    if (!result_info->GetInteger("status", &status_code))
      return Status(kUnknownError, "async result info has no int 'status'");
    if (status_code != kOk) {
      std::string message;
      result_info->GetString("value", &message);
      return Status(static_cast<StatusCode>(status_code), message);
    }

    base::Value* value = NULL;
    if (result_info->Get("value", &value)) {
      result->reset(value->DeepCopy());
      return Status(kOk);
    }

    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(100));
  }
}

Status WebViewImpl::IsNotPendingNavigation(const std::string& frame_id,
                                           bool* is_not_pending) {
  bool is_pending;
  Status status =
      navigation_tracker_->IsPendingNavigation(frame_id, &is_pending);
  if (status.IsError())
    return status;
  // An alert may block the pending navigation.
  if (is_pending && dialog_manager_->IsDialogOpen())
    return Status(kUnexpectedAlertOpen);

  *is_not_pending = !is_pending;
  return Status(kOk);
}

namespace internal {

Status EvaluateScript(DevToolsClient* client,
                      int context_id,
                      const std::string& expression,
                      EvaluateScriptReturnType return_type,
                      scoped_ptr<base::DictionaryValue>* result) {
  base::DictionaryValue params;
  params.SetString("expression", expression);
  if (context_id)
    params.SetInteger("contextId", context_id);
  params.SetBoolean("returnByValue", return_type == ReturnByValue);
  scoped_ptr<base::DictionaryValue> cmd_result;
  Status status = client->SendCommandAndGetResult(
      "Runtime.evaluate", params, &cmd_result);
  if (status.IsError())
    return status;

  bool was_thrown;
  if (!cmd_result->GetBoolean("wasThrown", &was_thrown))
    return Status(kUnknownError, "Runtime.evaluate missing 'wasThrown'");
  if (was_thrown) {
    std::string description = "unknown";
    cmd_result->GetString("result.description", &description);
    return Status(kUnknownError,
                  "Runtime.evaluate threw exception: " + description);
  }

  base::DictionaryValue* unscoped_result;
  if (!cmd_result->GetDictionary("result", &unscoped_result))
    return Status(kUnknownError, "evaluate missing dictionary 'result'");
  result->reset(unscoped_result->DeepCopy());
  return Status(kOk);
}

Status EvaluateScriptAndGetObject(DevToolsClient* client,
                                  int context_id,
                                  const std::string& expression,
                                  bool* got_object,
                                  std::string* object_id) {
  scoped_ptr<base::DictionaryValue> result;
  Status status = EvaluateScript(client, context_id, expression, ReturnByObject,
                                 &result);
  if (status.IsError())
    return status;
  if (!result->HasKey("objectId")) {
    *got_object = false;
    return Status(kOk);
  }
  if (!result->GetString("objectId", object_id))
    return Status(kUnknownError, "evaluate has invalid 'objectId'");
  *got_object = true;
  return Status(kOk);
}

Status EvaluateScriptAndGetValue(DevToolsClient* client,
                                 int context_id,
                                 const std::string& expression,
                                 scoped_ptr<base::Value>* result) {
  scoped_ptr<base::DictionaryValue> temp_result;
  Status status = EvaluateScript(client, context_id, expression, ReturnByValue,
                                 &temp_result);
  if (status.IsError())
    return status;

  std::string type;
  if (!temp_result->GetString("type", &type))
    return Status(kUnknownError, "Runtime.evaluate missing string 'type'");

  if (type == "undefined") {
    result->reset(base::Value::CreateNullValue());
  } else {
    base::Value* value;
    if (!temp_result->Get("value", &value))
      return Status(kUnknownError, "Runtime.evaluate missing 'value'");
    result->reset(value->DeepCopy());
  }
  return Status(kOk);
}

Status ParseCallFunctionResult(const base::Value& temp_result,
                               scoped_ptr<base::Value>* result) {
  const base::DictionaryValue* dict;
  if (!temp_result.GetAsDictionary(&dict))
    return Status(kUnknownError, "call function result must be a dictionary");
  int status_code;
  if (!dict->GetInteger("status", &status_code)) {
    return Status(kUnknownError,
                  "call function result missing int 'status'");
  }
  if (status_code != kOk) {
    std::string message;
    dict->GetString("value", &message);
    return Status(static_cast<StatusCode>(status_code), message);
  }
  const base::Value* unscoped_value;
  if (!dict->Get("value", &unscoped_value)) {
    return Status(kUnknownError,
                  "call function result missing 'value'");
  }
  result->reset(unscoped_value->DeepCopy());
  return Status(kOk);
}

Status GetNodeIdFromFunction(DevToolsClient* client,
                             int context_id,
                             const std::string& function,
                             const base::ListValue& args,
                             bool* found_node,
                             int* node_id) {
  std::string json;
  base::JSONWriter::Write(&args, &json);
  // TODO(zachconrad): Second null should be array of shadow host ids.
  std::string expression = base::StringPrintf(
      "(%s).apply(null, [null, %s, %s, true])",
      kCallFunctionScript,
      function.c_str(),
      json.c_str());

  bool got_object;
  std::string element_id;
  Status status = internal::EvaluateScriptAndGetObject(
      client, context_id, expression, &got_object, &element_id);
  if (status.IsError())
    return status;
  if (!got_object) {
    *found_node = false;
    return Status(kOk);
  }

  scoped_ptr<base::DictionaryValue> cmd_result;
  {
    base::DictionaryValue params;
    params.SetString("objectId", element_id);
    status = client->SendCommandAndGetResult(
        "DOM.requestNode", params, &cmd_result);
  }
  {
    // Release the remote object before doing anything else.
    base::DictionaryValue params;
    params.SetString("objectId", element_id);
    Status release_status =
        client->SendCommand("Runtime.releaseObject", params);
    if (release_status.IsError()) {
      LOG(ERROR) << "Failed to release remote object: "
                 << release_status.message();
    }
  }
  if (status.IsError())
    return status;

  if (!cmd_result->GetInteger("nodeId", node_id))
    return Status(kUnknownError, "DOM.requestNode missing int 'nodeId'");
  *found_node = true;
  return Status(kOk);
}

}  // namespace internal
