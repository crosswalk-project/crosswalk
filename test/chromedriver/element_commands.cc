// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/element_commands.h"

#include <list>
#include <vector>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/test/chromedriver/basic_types.h"
#include "chrome/test/chromedriver/chrome/chrome.h"
#include "chrome/test/chromedriver/chrome/js.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/ui_events.h"
#include "chrome/test/chromedriver/chrome/web_view.h"
#include "chrome/test/chromedriver/element_util.h"
#include "chrome/test/chromedriver/session.h"
#include "chrome/test/chromedriver/util.h"
#include "third_party/webdriver/atoms.h"

namespace {

Status SendKeysToElement(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const ListValue* key_list) {
  bool is_displayed = false;
  bool is_focused = false;
  base::TimeTicks start_time = base::TimeTicks::Now();
  while (true) {
    Status status = IsElementDisplayed(
        session, web_view, element_id, true, &is_displayed);
    if (status.IsError())
      return status;
    if (is_displayed)
      break;
    status = IsElementFocused(session, web_view, element_id, &is_focused);
    if (status.IsError())
      return status;
    if (is_focused)
      break;
    if (base::TimeTicks::Now() - start_time >= session->implicit_wait) {
      return Status(kElementNotVisible);
    }
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(100));
  }

  bool is_enabled = false;
  Status status = IsElementEnabled(session, web_view, element_id, &is_enabled);
  if (status.IsError())
    return status;
  if (!is_enabled)
    return Status(kInvalidElementState);

  if (!is_focused) {
    base::ListValue args;
    args.Append(CreateElement(element_id));
    scoped_ptr<base::Value> result;
    status = web_view->CallFunction(
        session->GetCurrentFrameId(), kFocusScript, args, &result);
    if (status.IsError())
      return status;
  }

  return SendKeysOnWindow(web_view, key_list, true, &session->sticky_modifiers);
}

Status ExecuteTouchSingleTapAtom(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::TOUCH_SINGLE_TAP),
      args,
      value);
}

}  // namespace

Status ExecuteElementCommand(
    const ElementCommand& command,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string id;
  if (params.GetString("id", &id) || params.GetString("element", &id))
    return command.Run(session, web_view, id, params, value);
  return Status(kUnknownError, "element identifier must be a string");
}

Status ExecuteFindChildElement(
    int interval_ms,
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return FindElement(
      interval_ms, true, &element_id, session, web_view, params, value);
}

Status ExecuteFindChildElements(
    int interval_ms,
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return FindElement(
      interval_ms, false, &element_id, session, web_view, params, value);
}

Status ExecuteHoverOverElement(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  WebPoint location;
  Status status = GetElementClickableLocation(
      session, web_view, element_id, &location);
  if (status.IsError())
    return status;

  MouseEvent move_event(
      kMovedMouseEventType, kNoneMouseButton, location.x, location.y,
      session->sticky_modifiers, 0);
  std::list<MouseEvent> events;
  events.push_back(move_event);
  status = web_view->DispatchMouseEvents(events, session->GetCurrentFrameId());
  if (status.IsOk())
    session->mouse_position = location;
  return status;
}

Status ExecuteClickElement(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string tag_name;
  Status status = GetElementTagName(session, web_view, element_id, &tag_name);
  if (status.IsError())
    return status;
  if (tag_name == "option") {
    bool is_toggleable;
    status = IsOptionElementTogglable(
        session, web_view, element_id, &is_toggleable);
    if (status.IsError())
      return status;
    if (is_toggleable)
      return ToggleOptionElement(session, web_view, element_id);
    else
      return SetOptionElementSelected(session, web_view, element_id, true);
  } else {
    WebPoint location;
    status = GetElementClickableLocation(
        session, web_view, element_id, &location);
    if (status.IsError())
      return status;

    std::list<MouseEvent> events;
    events.push_back(
        MouseEvent(kMovedMouseEventType, kNoneMouseButton,
                   location.x, location.y, session->sticky_modifiers, 0));
    events.push_back(
        MouseEvent(kPressedMouseEventType, kLeftMouseButton,
                   location.x, location.y, session->sticky_modifiers, 1));
    events.push_back(
        MouseEvent(kReleasedMouseEventType, kLeftMouseButton,
                   location.x, location.y, session->sticky_modifiers, 1));
    status =
        web_view->DispatchMouseEvents(events, session->GetCurrentFrameId());
    if (status.IsOk())
      session->mouse_position = location;
    return status;
  }
}

Status ExecuteTouchSingleTap(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  // Fall back to javascript atom for pre-m30 Chrome.
  if (session->chrome->GetBuildNo() < 1576)
    return ExecuteTouchSingleTapAtom(
        session, web_view, element_id, params, value);

  WebPoint location;
  Status status = GetElementClickableLocation(
      session, web_view, element_id, &location);
  if (status.IsError())
    return status;

  std::list<TouchEvent> events;
  events.push_back(
      TouchEvent(kTouchStart, location.x, location.y));
  events.push_back(
      TouchEvent(kTouchEnd, location.x, location.y));
  return web_view->DispatchTouchEvents(events);
}

Status ExecuteClearElement(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  scoped_ptr<base::Value> result;
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::CLEAR),
      args, &result);
}

Status ExecuteSendKeysToElement(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const base::ListValue* key_list;
  if (!params.GetList("value", &key_list))
    return Status(kUnknownError, "'value' must be a list");

  bool is_input = false;
  Status status = IsElementAttributeEqualToIgnoreCase(
      session, web_view, element_id, "tagName", "input", &is_input);
  if (status.IsError())
    return status;
  bool is_file = false;
  status = IsElementAttributeEqualToIgnoreCase(
      session, web_view, element_id, "type", "file", &is_file);
  if (status.IsError())
    return status;
  if (is_input && is_file) {
    // Compress array into a single string.
    base::FilePath::StringType paths_string;
    for (size_t i = 0; i < key_list->GetSize(); ++i) {
      base::FilePath::StringType path_part;
      if (!key_list->GetString(i, &path_part))
        return Status(kUnknownError, "'value' is invalid");
      paths_string.append(path_part);
    }

    // Separate the string into separate paths, delimited by '\n'.
    std::vector<base::FilePath::StringType> path_strings;
    base::SplitString(paths_string, '\n', &path_strings);
    std::vector<base::FilePath> paths;
    for (size_t i = 0; i < path_strings.size(); ++i)
      paths.push_back(base::FilePath(path_strings[i]));

    bool multiple = false;
    status = IsElementAttributeEqualToIgnoreCase(
        session, web_view, element_id, "multiple", "true", &multiple);
    if (status.IsError())
      return status;
    if (!multiple && paths.size() > 1)
      return Status(kUnknownError, "the element can not hold multiple files");

    scoped_ptr<base::DictionaryValue> element(CreateElement(element_id));
    return web_view->SetFileInputFiles(
        session->GetCurrentFrameId(), *element, paths);
  } else {
    return SendKeysToElement(session, web_view, element_id, key_list);
  }
}

Status ExecuteSubmitElement(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::SUBMIT),
      args,
      value);
}

Status ExecuteGetElementText(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::GET_TEXT),
      args,
      value);
}

Status ExecuteGetElementValue(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      "function(elem) { return elem['value'] }",
      args,
      value);
}

Status ExecuteGetElementTagName(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      "function(elem) { return elem.tagName.toLowerCase() }",
      args,
      value);
}

Status ExecuteIsElementSelected(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::IS_SELECTED),
      args,
      value);
}

Status ExecuteIsElementEnabled(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::IS_ENABLED),
      args,
      value);
}

Status ExecuteIsElementDisplayed(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::IS_DISPLAYED),
      args,
      value);
}

Status ExecuteGetElementLocation(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::GET_LOCATION),
      args,
      value);
}

Status ExecuteGetElementLocationOnceScrolledIntoView(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  WebPoint location;
  Status status = ScrollElementIntoView(
      session, web_view, element_id, &location);
  if (status.IsError())
    return status;
  value->reset(CreateValueFrom(location));
  return Status(kOk);
}

Status ExecuteGetElementSize(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      webdriver::atoms::asString(webdriver::atoms::GET_SIZE),
      args,
      value);
}

Status ExecuteGetElementAttribute(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string name;
  if (!params.GetString("name", &name))
    return Status(kUnknownError, "missing 'name'");
  return GetElementAttribute(session, web_view, element_id, name, value);
}

Status ExecuteGetElementValueOfCSSProperty(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string property_name;
  if (!params.GetString("propertyName", &property_name))
    return Status(kUnknownError, "missing 'propertyName'");
  std::string property_value;
  Status status = GetElementEffectiveStyle(
      session, web_view, element_id, property_name, &property_value);
  if (status.IsError())
    return status;
  value->reset(new base::StringValue(property_value));
  return Status(kOk);
}

Status ExecuteElementEquals(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string other_element_id;
  if (!params.GetString("other", &other_element_id))
    return Status(kUnknownError, "'other' must be a string");
  value->reset(new base::FundamentalValue(element_id == other_element_id));
  return Status(kOk);
}
