// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/element_util.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/test/chromedriver/basic_types.h"
#include "chrome/test/chromedriver/chrome/js.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/web_view.h"
#include "chrome/test/chromedriver/session.h"
#include "third_party/webdriver/atoms.h"

namespace {

const char kElementKey[] = "ELEMENT";

bool ParseFromValue(base::Value* value, WebPoint* point) {
  base::DictionaryValue* dict_value;
  if (!value->GetAsDictionary(&dict_value))
    return false;
  double x, y;
  if (!dict_value->GetDouble("x", &x) ||
      !dict_value->GetDouble("y", &y))
    return false;
  point->x = static_cast<int>(x);
  point->y = static_cast<int>(y);
  return true;
}

bool ParseFromValue(base::Value* value, WebSize* size) {
  base::DictionaryValue* dict_value;
  if (!value->GetAsDictionary(&dict_value))
    return false;
  double width, height;
  if (!dict_value->GetDouble("width", &width) ||
      !dict_value->GetDouble("height", &height))
    return false;
  size->width = static_cast<int>(width);
  size->height = static_cast<int>(height);
  return true;
}

base::Value* CreateValueFrom(const WebSize& size) {
  base::DictionaryValue* dict = new base::DictionaryValue();
  dict->SetInteger("width", size.width);
  dict->SetInteger("height", size.height);
  return dict;
}

bool ParseFromValue(base::Value* value, WebRect* rect) {
  base::DictionaryValue* dict_value;
  if (!value->GetAsDictionary(&dict_value))
    return false;
  double x, y, width, height;
  if (!dict_value->GetDouble("left", &x) ||
      !dict_value->GetDouble("top", &y) ||
      !dict_value->GetDouble("width", &width) ||
      !dict_value->GetDouble("height", &height))
    return false;
  rect->origin.x = static_cast<int>(x);
  rect->origin.y = static_cast<int>(y);
  rect->size.width = static_cast<int>(width);
  rect->size.height = static_cast<int>(height);
  return true;
}

base::Value* CreateValueFrom(const WebRect& rect) {
  base::DictionaryValue* dict = new base::DictionaryValue();
  dict->SetInteger("left", rect.X());
  dict->SetInteger("top", rect.Y());
  dict->SetInteger("width", rect.Width());
  dict->SetInteger("height", rect.Height());
  return dict;
}

Status CallAtomsJs(
    const std::string& frame,
    WebView* web_view,
    const char* const* atom_function,
    const base::ListValue& args,
    scoped_ptr<base::Value>* result) {
  return web_view->CallFunction(
      frame, webdriver::atoms::asString(atom_function), args, result);
}

Status VerifyElementClickable(
    const std::string& frame,
    WebView* web_view,
    const std::string& element_id,
    const WebPoint& location) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  args.Append(CreateValueFrom(location));
  scoped_ptr<base::Value> result;
  Status status = CallAtomsJs(
      frame, web_view, webdriver::atoms::IS_ELEMENT_CLICKABLE,
      args, &result);
  if (status.IsError())
    return status;
  base::DictionaryValue* dict;
  bool is_clickable;
  if (!result->GetAsDictionary(&dict) ||
      !dict->GetBoolean("clickable", &is_clickable)) {
    return Status(kUnknownError,
                  "failed to parse value of IS_ELEMENT_CLICKABLE");
  }

  if (!is_clickable) {
    std::string message;
    if (!dict->GetString("message", &message))
      message = "element is not clickable";
    return Status(kUnknownError, message);
  }
  return Status(kOk);
}

Status ScrollElementRegionIntoViewHelper(
    const std::string& frame,
    WebView* web_view,
    const std::string& element_id,
    const WebRect& region,
    bool center,
    const std::string& clickable_element_id,
    WebPoint* location) {
  WebPoint tmp_location = *location;
  base::ListValue args;
  args.Append(CreateElement(element_id));
  args.AppendBoolean(center);
  args.Append(CreateValueFrom(region));
  scoped_ptr<base::Value> result;
  Status status = web_view->CallFunction(
      frame, webdriver::atoms::asString(webdriver::atoms::GET_LOCATION_IN_VIEW),
      args, &result);
  if (status.IsError())
    return status;
  if (!ParseFromValue(result.get(), &tmp_location)) {
    return Status(kUnknownError,
                  "failed to parse value of GET_LOCATION_IN_VIEW");
  }
  if (!clickable_element_id.empty()) {
    WebPoint middle = tmp_location;
    middle.Offset(region.Width() / 2, region.Height() / 2);
    status = VerifyElementClickable(
        frame, web_view, clickable_element_id, middle);
    if (status.IsError())
      return status;
  }
  *location = tmp_location;
  return Status(kOk);
}

Status GetElementEffectiveStyle(
    const std::string& frame,
    WebView* web_view,
    const std::string& element_id,
    const std::string& property,
    std::string* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  args.AppendString(property);
  scoped_ptr<base::Value> result;
  Status status = web_view->CallFunction(
      frame, webdriver::atoms::asString(webdriver::atoms::GET_EFFECTIVE_STYLE),
      args, &result);
  if (status.IsError())
    return status;
  if (!result->GetAsString(value)) {
    return Status(kUnknownError,
                  "failed to parse value of GET_EFFECTIVE_STYLE");
  }
  return Status(kOk);
}

Status GetElementBorder(
    const std::string& frame,
    WebView* web_view,
    const std::string& element_id,
    int* border_left,
    int* border_top) {
  std::string border_left_str;
  Status status = GetElementEffectiveStyle(
      frame, web_view, element_id, "border-left-width", &border_left_str);
  if (status.IsError())
    return status;
  std::string border_top_str;
  status = GetElementEffectiveStyle(
      frame, web_view, element_id, "border-top-width", &border_top_str);
  if (status.IsError())
    return status;
  int border_left_tmp = -1;
  int border_top_tmp = -1;
  base::StringToInt(border_left_str, &border_left_tmp);
  base::StringToInt(border_top_str, &border_top_tmp);
  if (border_left_tmp == -1 || border_top_tmp == -1)
    return Status(kUnknownError, "failed to get border width of element");
  *border_left = border_left_tmp;
  *border_top = border_top_tmp;
  return Status(kOk);
}

}  // namespace

base::DictionaryValue* CreateElement(const std::string& element_id) {
  base::DictionaryValue* element = new base::DictionaryValue();
  element->SetString(kElementKey, element_id);
  return element;
}

base::Value* CreateValueFrom(const WebPoint& point) {
  base::DictionaryValue* dict = new base::DictionaryValue();
  dict->SetInteger("x", point.x);
  dict->SetInteger("y", point.y);
  return dict;
}

Status FindElement(
    int interval_ms,
    bool only_one,
    const std::string* root_element_id,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string strategy;
  if (!params.GetString("using", &strategy))
    return Status(kUnknownError, "'using' must be a string");
  std::string target;
  if (!params.GetString("value", &target))
    return Status(kUnknownError, "'value' must be a string");

  std::string script;
  if (only_one)
    script = webdriver::atoms::asString(webdriver::atoms::FIND_ELEMENT);
  else
    script = webdriver::atoms::asString(webdriver::atoms::FIND_ELEMENTS);
  scoped_ptr<base::DictionaryValue> locator(new base::DictionaryValue());
  locator->SetString(strategy, target);
  base::ListValue arguments;
  arguments.Append(locator.release());
  if (root_element_id)
    arguments.Append(CreateElement(*root_element_id));

  base::TimeTicks start_time = base::TimeTicks::Now();
  while (true) {
    scoped_ptr<base::Value> temp;
    Status status = web_view->CallFunction(
        session->GetCurrentFrameId(), script, arguments, &temp);
    if (status.IsError())
      return status;

    if (!temp->IsType(base::Value::TYPE_NULL)) {
      if (only_one) {
        value->reset(temp.release());
        return Status(kOk);
      } else {
        base::ListValue* result;
        if (!temp->GetAsList(&result))
          return Status(kUnknownError, "script returns unexpected result");

        if (result->GetSize() > 0U) {
          value->reset(temp.release());
          return Status(kOk);
        }
      }
    }

    if (base::TimeTicks::Now() - start_time >= session->implicit_wait) {
      if (only_one) {
        return Status(kNoSuchElement);
      } else {
        value->reset(new base::ListValue());
        return Status(kOk);
      }
    }
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(interval_ms));
  }

  return Status(kUnknownError);
}

Status GetActiveElement(
    Session* session,
    WebView* web_view,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      "function() { return document.activeElement || document.body }",
      args,
      value);
}

Status IsElementFocused(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    bool* is_focused) {
  scoped_ptr<base::Value> result;
  Status status = GetActiveElement(session, web_view, &result);
  if (status.IsError())
    return status;
  scoped_ptr<base::Value> element_dict(CreateElement(element_id));
  *is_focused = result->Equals(element_dict.get());
  return Status(kOk);
}

Status GetElementAttribute(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const std::string& attribute_name,
    scoped_ptr<base::Value>* value) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  args.AppendString(attribute_name);
  return CallAtomsJs(
      session->GetCurrentFrameId(), web_view, webdriver::atoms::GET_ATTRIBUTE,
      args, value);
}

Status IsElementAttributeEqualToIgnoreCase(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const std::string& attribute_name,
    const std::string& attribute_value,
    bool* is_equal) {
  scoped_ptr<base::Value> result;
  Status status = GetElementAttribute(
      session, web_view, element_id, attribute_name, &result);
  if (status.IsError())
    return status;
  std::string actual_value;
  if (result->GetAsString(&actual_value))
    *is_equal = LowerCaseEqualsASCII(actual_value, attribute_value.c_str());
  else
    *is_equal = false;
  return status;
}

Status GetElementClickableLocation(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    WebPoint* location) {
  std::string tag_name;
  Status status = GetElementTagName(session, web_view, element_id, &tag_name);
  if (status.IsError())
    return status;
  std::string target_element_id = element_id;
  if (tag_name == "area") {
    // Scroll the image into view instead of the area.
    const char* kGetImageElementForArea =
        "function (element) {"
        "  var map = element.parentElement;"
        "  if (map.tagName.toLowerCase() != 'map')"
        "    throw new Error('the area is not within a map');"
        "  var mapName = map.getAttribute('name');"
        "  if (mapName == null)"
        "    throw new Error ('area\\'s parent map must have a name');"
        "  mapName = '#' + mapName.toLowerCase();"
        "  var images = document.getElementsByTagName('img');"
        "  for (var i = 0; i < images.length; i++) {"
        "    if (images[i].useMap.toLowerCase() == mapName)"
        "      return images[i];"
        "  }"
        "  throw new Error('no img is found for the area');"
        "}";
    base::ListValue args;
    args.Append(CreateElement(element_id));
    scoped_ptr<base::Value> result;
    status = web_view->CallFunction(
        session->GetCurrentFrameId(), kGetImageElementForArea, args, &result);
    if (status.IsError())
      return status;
    const base::DictionaryValue* element_dict;
    if (!result->GetAsDictionary(&element_dict) ||
        !element_dict->GetString(kElementKey, &target_element_id))
      return Status(kUnknownError, "no element reference returned by script");
  }
  bool is_displayed = false;
  status = IsElementDisplayed(
      session, web_view, target_element_id, true, &is_displayed);
  if (status.IsError())
    return status;
  if (!is_displayed)
    return Status(kElementNotVisible);

  WebRect rect;
  status = GetElementRegion(session, web_view, element_id, &rect);
  if (status.IsError())
    return status;

  status = ScrollElementRegionIntoView(
      session, web_view, target_element_id, rect,
      true /* center */, element_id, location);
  if (status.IsError())
    return status;
  location->Offset(rect.Width() / 2, rect.Height() / 2);
  return Status(kOk);
}

Status GetElementEffectiveStyle(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const std::string& property_name,
    std::string* property_value) {
  return GetElementEffectiveStyle(session->GetCurrentFrameId(), web_view,
                                  element_id, property_name, property_value);
}

Status GetElementRegion(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    WebRect* rect) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  scoped_ptr<base::Value> result;
  Status status = web_view->CallFunction(
      session->GetCurrentFrameId(), kGetElementRegionScript, args, &result);
  if (status.IsError())
    return status;
  if (!ParseFromValue(result.get(), rect)) {
    return Status(kUnknownError,
                  "failed to parse value of getElementRegion");
  }
  return Status(kOk);
}

Status GetElementTagName(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    std::string* name) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  scoped_ptr<base::Value> result;
  Status status = web_view->CallFunction(
      session->GetCurrentFrameId(),
      "function(elem) { return elem.tagName.toLowerCase(); }",
      args, &result);
  if (status.IsError())
    return status;
  if (!result->GetAsString(name))
    return Status(kUnknownError, "failed to get element tag name");
  return Status(kOk);
}

Status GetElementSize(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    WebSize* size) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  scoped_ptr<base::Value> result;
  Status status = CallAtomsJs(
      session->GetCurrentFrameId(), web_view, webdriver::atoms::GET_SIZE,
      args, &result);
  if (status.IsError())
    return status;
  if (!ParseFromValue(result.get(), size))
    return Status(kUnknownError, "failed to parse value of GET_SIZE");
  return Status(kOk);
}

Status IsElementDisplayed(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    bool ignore_opacity,
    bool* is_displayed) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  args.AppendBoolean(ignore_opacity);
  scoped_ptr<base::Value> result;
  Status status = CallAtomsJs(
      session->GetCurrentFrameId(), web_view, webdriver::atoms::IS_DISPLAYED,
      args, &result);
  if (status.IsError())
    return status;
  if (!result->GetAsBoolean(is_displayed))
    return Status(kUnknownError, "IS_DISPLAYED should return a boolean value");
  return Status(kOk);
}

Status IsElementEnabled(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    bool* is_enabled) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  scoped_ptr<base::Value> result;
  Status status = CallAtomsJs(
      session->GetCurrentFrameId(), web_view, webdriver::atoms::IS_ENABLED,
      args, &result);
  if (status.IsError())
    return status;
  if (!result->GetAsBoolean(is_enabled))
    return Status(kUnknownError, "IS_ENABLED should return a boolean value");
  return Status(kOk);
}

Status IsOptionElementSelected(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    bool* is_selected) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  scoped_ptr<base::Value> result;
  Status status = CallAtomsJs(
      session->GetCurrentFrameId(), web_view, webdriver::atoms::IS_SELECTED,
      args, &result);
  if (status.IsError())
    return status;
  if (!result->GetAsBoolean(is_selected))
    return Status(kUnknownError, "IS_SELECTED should return a boolean value");
  return Status(kOk);
}

Status IsOptionElementTogglable(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    bool* is_togglable) {
  base::ListValue args;
  args.Append(CreateElement(element_id));
  scoped_ptr<base::Value> result;
  Status status = web_view->CallFunction(
      session->GetCurrentFrameId(), kIsOptionElementToggleableScript,
      args, &result);
  if (status.IsError())
    return status;
  if (!result->GetAsBoolean(is_togglable))
    return Status(kUnknownError, "failed check if option togglable or not");
  return Status(kOk);
}

Status SetOptionElementSelected(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    bool selected) {
  // TODO(171034): need to fix throwing error if an alert is triggered.
  base::ListValue args;
  args.Append(CreateElement(element_id));
  args.AppendBoolean(selected);
  scoped_ptr<base::Value> result;
  return CallAtomsJs(
      session->GetCurrentFrameId(), web_view, webdriver::atoms::CLICK,
      args, &result);
}

Status ToggleOptionElement(
    Session* session,
    WebView* web_view,
    const std::string& element_id) {
  bool is_selected;
  Status status = IsOptionElementSelected(
      session, web_view, element_id, &is_selected);
  if (status.IsError())
    return status;
  return SetOptionElementSelected(session, web_view, element_id, !is_selected);
}

Status ScrollElementIntoView(
    Session* session,
    WebView* web_view,
    const std::string& id,
    WebPoint* location) {
  WebSize size;
  Status status = GetElementSize(session, web_view, id, &size);
  if (status.IsError())
    return status;
  return ScrollElementRegionIntoView(
      session, web_view, id, WebRect(WebPoint(0, 0), size),
      false /* center */, std::string(), location);
}

Status ScrollElementRegionIntoView(
    Session* session,
    WebView* web_view,
    const std::string& element_id,
    const WebRect& region,
    bool center,
    const std::string& clickable_element_id,
    WebPoint* location) {
  WebPoint region_offset = region.origin;
  WebSize region_size = region.size;
  Status status = ScrollElementRegionIntoViewHelper(
      session->GetCurrentFrameId(), web_view, element_id, region,
      center, clickable_element_id, &region_offset);
  if (status.IsError())
    return status;
  const char* kFindSubFrameScript =
      "function(xpath) {"
      "  return document.evaluate(xpath, document, null,"
      "      XPathResult.FIRST_ORDERED_NODE_TYPE, null).singleNodeValue;"
      "}";
  for (std::list<FrameInfo>::reverse_iterator rit = session->frames.rbegin();
       rit != session->frames.rend(); ++rit) {
    base::ListValue args;
    args.AppendString(
        base::StringPrintf("//*[@cd_frame_id_ = '%s']",
                           rit->chromedriver_frame_id.c_str()));
    scoped_ptr<base::Value> result;
    status = web_view->CallFunction(
        rit->parent_frame_id, kFindSubFrameScript, args, &result);
    if (status.IsError())
      return status;
    const base::DictionaryValue* element_dict;
    if (!result->GetAsDictionary(&element_dict))
      return Status(kUnknownError, "no element reference returned by script");
    std::string frame_element_id;
    if (!element_dict->GetString(kElementKey, &frame_element_id))
      return Status(kUnknownError, "failed to locate a sub frame");

    // Modify |region_offset| by the frame's border.
    int border_left = -1;
    int border_top = -1;
    status = GetElementBorder(
        rit->parent_frame_id, web_view, frame_element_id,
        &border_left, &border_top);
    if (status.IsError())
      return status;
    region_offset.Offset(border_left, border_top);

    status = ScrollElementRegionIntoViewHelper(
        rit->parent_frame_id, web_view, frame_element_id,
        WebRect(region_offset, region_size),
        center, frame_element_id, &region_offset);
    if (status.IsError())
      return status;
  }
  *location = region_offset;
  return Status(kOk);
}
