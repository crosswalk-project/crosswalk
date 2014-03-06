// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/window_commands.h"

#include <list>
#include <string>

#include "base/callback.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/basic_types.h"
#include "xwalk/test/chromedriver/chrome/chrome.h"
#include "xwalk/test/chromedriver/chrome/xwalk_desktop_impl.h"
#include "xwalk/test/chromedriver/chrome/devtools_client.h"
#include "xwalk/test/chromedriver/chrome/geoposition.h"
#include "xwalk/test/chromedriver/chrome/javascript_dialog_manager.h"
#include "xwalk/test/chromedriver/chrome/js.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/ui_events.h"
#include "xwalk/test/chromedriver/chrome/web_view.h"
#include "xwalk/test/chromedriver/element_util.h"
#include "xwalk/test/chromedriver/session.h"
#include "xwalk/test/chromedriver/util.h"

namespace {

Status GetMouseButton(const base::DictionaryValue& params,
                      MouseButton* button) {
  int button_num;
  if (!params.GetInteger("button", &button_num)) {
    button_num = 0;  // Default to left mouse button.
  } else if (button_num < 0 || button_num > 2) {
    return Status(kUnknownError,
                  base::StringPrintf("invalid button: %d", button_num));
  }
  *button = static_cast<MouseButton>(button_num);
  return Status(kOk);
}

Status GetUrl(WebView* web_view, const std::string& frame, std::string* url) {
  scoped_ptr<base::Value> value;
  base::ListValue args;
  Status status = web_view->CallFunction(
      frame, "function() { return document.URL; }", args, &value);
  if (status.IsError())
    return status;
  if (!value->GetAsString(url))
    return Status(kUnknownError, "javascript failed to return the url");
  return Status(kOk);
}

struct Cookie {
  Cookie(const std::string& name,
         const std::string& value,
         const std::string& domain,
         const std::string& path,
         double expiry,
         bool secure,
         bool session)
      : name(name), value(value), domain(domain), path(path), expiry(expiry),
        secure(secure), session(session) {}

  std::string name;
  std::string value;
  std::string domain;
  std::string path;
  double expiry;
  bool secure;
  bool session;
};

base::DictionaryValue* CreateDictionaryFrom(const Cookie& cookie) {
  base::DictionaryValue* dict = new base::DictionaryValue();
  dict->SetString("name", cookie.name);
  dict->SetString("value", cookie.value);
  if (!cookie.domain.empty())
    dict->SetString("domain", cookie.domain);
  if (!cookie.path.empty())
    dict->SetString("path", cookie.path);
  if (!cookie.session)
    dict->SetDouble("expiry", cookie.expiry);
  dict->SetBoolean("secure", cookie.secure);
  return dict;
}

Status GetVisibleCookies(WebView* web_view,
                         std::list<Cookie>* cookies) {
  scoped_ptr<base::ListValue> internal_cookies;
  Status status = web_view->GetCookies(&internal_cookies);
  if (status.IsError())
    return status;
  std::list<Cookie> cookies_tmp;
  for (size_t i = 0; i < internal_cookies->GetSize(); ++i) {
    base::DictionaryValue* cookie_dict;
    if (!internal_cookies->GetDictionary(i, &cookie_dict))
      return Status(kUnknownError, "DevTools returns a non-dictionary cookie");

    std::string name;
    cookie_dict->GetString("name", &name);
    std::string value;
    cookie_dict->GetString("value", &value);
    std::string domain;
    cookie_dict->GetString("domain", &domain);
    std::string path;
    cookie_dict->GetString("path", &path);
    double expiry = 0;
    cookie_dict->GetDouble("expires", &expiry);
    expiry /= 1000;  // Convert from millisecond to second.
    bool session = false;
    cookie_dict->GetBoolean("session", &session);
    bool secure = false;
    cookie_dict->GetBoolean("secure", &secure);

    cookies_tmp.push_back(
        Cookie(name, value, domain, path, expiry, secure, session));
  }
  cookies->swap(cookies_tmp);
  return Status(kOk);
}

Status ScrollCoordinateInToView(
    Session* session, WebView* web_view, int x, int y, int* offset_x,
    int* offset_y) {
  scoped_ptr<base::Value> value;
  base::ListValue args;
  args.AppendInteger(x);
  args.AppendInteger(y);
  Status status = web_view->CallFunction(
      std::string(),
      "function(x, y) {"
      "  if (x < window.pageXOffset ||"
      "      x >= window.pageXOffset + window.innerWidth ||"
      "      y < window.pageYOffset ||"
      "      y >= window.pageYOffset + window.innerHeight) {"
      "    window.scrollTo(x - window.innerWidth/2, y - window.innerHeight/2);"
      "  }"
      "  return {"
      "    view_x: Math.floor(window.pageXOffset),"
      "    view_y: Math.floor(window.pageYOffset),"
      "    view_width: Math.floor(window.innerWidth),"
      "    view_height: Math.floor(window.innerHeight)};"
      "}",
      args,
      &value);
  if (!status.IsOk())
    return status;
  base::DictionaryValue* view_attrib;
  value->GetAsDictionary(&view_attrib);
  int view_x, view_y, view_width, view_height;
  view_attrib->GetInteger("view_x", &view_x);
  view_attrib->GetInteger("view_y", &view_y);
  view_attrib->GetInteger("view_width", &view_width);
  view_attrib->GetInteger("view_height", &view_height);
  *offset_x = x - view_x;
  *offset_y = y - view_y;
  if (*offset_x < 0 || *offset_x >= view_width || *offset_y < 0 ||
      *offset_y >= view_height)
    return Status(kUnknownError, "Failed to scroll coordinate into view");
  return Status(kOk);
}

Status ExecuteTouchEvent(
    Session* session, WebView* web_view, TouchEventType type,
    const base::DictionaryValue& params) {
  int x, y;
  if (!params.GetInteger("x", &x))
    return Status(kUnknownError, "'x' must be an integer");
  if (!params.GetInteger("y", &y))
    return Status(kUnknownError, "'y' must be an integer");
  int relative_x = x;
  int relative_y = y;
  Status status = ScrollCoordinateInToView(
      session, web_view, x, y, &relative_x, &relative_y);
  if (!status.IsOk())
    return status;
  std::list<TouchEvent> events;
  events.push_back(
      TouchEvent(type, relative_x, relative_y));
  return web_view->DispatchTouchEvents(events);
}

}  // namespace

Status ExecuteWindowCommand(
    const WindowCommand& command,
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  WebView* web_view = NULL;
  Status status = session->GetTargetWindow(&web_view);
  if (status.IsError())
    return status;

  status = web_view->ConnectIfNecessary();
  if (status.IsError())
    return status;

  status = web_view->HandleReceivedEvents();
  if (status.IsError())
    return status;

  if (web_view->GetJavaScriptDialogManager()->IsDialogOpen())
    return Status(kUnexpectedAlertOpen);

  Status nav_status(kOk);
  for (int attempt = 0; attempt < 2; attempt++) {
    if (attempt == 1) {
      if (status.code() == kNoSuchExecutionContext)
        // Switch to main frame and retry command if subframe no longer exists.
        session->SwitchToTopFrame();
      else
        break;
    }
    nav_status = web_view->WaitForPendingNavigations(
        session->GetCurrentFrameId(), session->page_load_timeout, true);
    if (nav_status.IsError())
      return nav_status;

    status = command.Run(session, web_view, params, value);
  }

  nav_status = web_view->WaitForPendingNavigations(
      session->GetCurrentFrameId(), session->page_load_timeout, true);

  if (status.IsOk() && nav_status.IsError() &&
      nav_status.code() != kUnexpectedAlertOpen)
    return nav_status;
  if (status.code() == kUnexpectedAlertOpen)
    return Status(kOk);
  return status;
}

Status ExecuteGet(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string url;
  if (!params.GetString("url", &url))
    return Status(kUnknownError, "'url' must be a string");
  return web_view->Load(url);
}

Status ExecuteExecuteScript(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string script;
  if (!params.GetString("script", &script))
    return Status(kUnknownError, "'script' must be a string");
  if (script == ":takeHeapSnapshot") {
    return web_view->TakeHeapSnapshot(value);
  } else {
    const base::ListValue* args;
    if (!params.GetList("args", &args))
      return Status(kUnknownError, "'args' must be a list");

    return web_view->CallFunction(session->GetCurrentFrameId(),
                                  "function(){" + script + "}", *args, value);
  }
}

Status ExecuteExecuteAsyncScript(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string script;
  if (!params.GetString("script", &script))
    return Status(kUnknownError, "'script' must be a string");
  const base::ListValue* args;
  if (!params.GetList("args", &args))
    return Status(kUnknownError, "'args' must be a list");

  return web_view->CallUserAsyncFunction(
      session->GetCurrentFrameId(), "function(){" + script + "}", *args,
      session->script_timeout, value);
}

Status ExecuteSwitchToFrame(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const base::Value* id;
  if (!params.Get("id", &id))
    return Status(kUnknownError, "missing 'id'");

  if (id->IsType(base::Value::TYPE_NULL)) {
    session->SwitchToTopFrame();
    return Status(kOk);
  }

  std::string script;
  base::ListValue args;
  const base::DictionaryValue* id_dict;
  if (id->GetAsDictionary(&id_dict)) {
    script = "function(elem) { return elem; }";
    args.Append(id_dict->DeepCopy());
  } else {
    script =
        "function(xpath) {"
        "  return document.evaluate(xpath, document, null, "
        "      XPathResult.FIRST_ORDERED_NODE_TYPE, null).singleNodeValue;"
        "}";
    std::string xpath = "(/html/body//iframe|/html/frameset/frame)";
    std::string id_string;
    int id_int;
    if (id->GetAsString(&id_string)) {
      xpath += base::StringPrintf(
          "[@name=\"%s\" or @id=\"%s\"]", id_string.c_str(), id_string.c_str());
    } else if (id->GetAsInteger(&id_int)) {
      xpath += base::StringPrintf("[%d]", id_int + 1);
    } else {
      return Status(kUnknownError, "invalid 'id'");
    }
    args.Append(new base::StringValue(xpath));
  }
  std::string frame;
  Status status = web_view->GetFrameByFunction(
      session->GetCurrentFrameId(), script, args, &frame);
  if (status.IsError())
    return status;

  scoped_ptr<base::Value> result;
  status = web_view->CallFunction(
      session->GetCurrentFrameId(), script, args, &result);
  if (status.IsError())
    return status;
  const base::DictionaryValue* element;
  if (!result->GetAsDictionary(&element))
    return Status(kUnknownError, "fail to locate the sub frame element");

  std::string chrome_driver_id = GenerateId();
  const char* kSetFrameIdentifier =
      "function(frame, id) {"
      "  frame.setAttribute('cd_frame_id_', id);"
      "}";
  base::ListValue new_args;
  new_args.Append(element->DeepCopy());
  new_args.AppendString(chrome_driver_id);
  result.reset(NULL);
  status = web_view->CallFunction(
      session->GetCurrentFrameId(), kSetFrameIdentifier, new_args, &result);
  if (status.IsError())
    return status;
  session->SwitchToSubFrame(frame, chrome_driver_id);
  return Status(kOk);
}

Status ExecuteGetTitle(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const char* kGetTitleScript =
      "function() {"
      "  if (document.title)"
      "    return document.title;"
      "  else"
      "    return document.URL;"
      "}";
  base::ListValue args;
  return web_view->CallFunction(std::string(), kGetTitleScript, args, value);
}

Status ExecuteGetPageSource(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const char* kGetPageSource =
      "function() {"
      "  return new XMLSerializer().serializeToString(document);"
      "}";
  base::ListValue args;
  return web_view->CallFunction(
      session->GetCurrentFrameId(), kGetPageSource, args, value);
}

Status ExecuteFindElement(
    int interval_ms,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return FindElement(interval_ms, true, NULL, session, web_view, params, value);
}

Status ExecuteFindElements(
    int interval_ms,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return FindElement(
      interval_ms, false, NULL, session, web_view, params, value);
}

Status ExecuteGetCurrentUrl(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string url;
  Status status = GetUrl(web_view, session->GetCurrentFrameId(), &url);
  if (status.IsError())
    return status;
  value->reset(new base::StringValue(url));
  return Status(kOk);
}

Status ExecuteGoBack(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->EvaluateScript(
      std::string(), "window.history.back();", value);
}

Status ExecuteGoForward(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->EvaluateScript(
      std::string(), "window.history.forward();", value);
}

Status ExecuteRefresh(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->Reload();
}

Status ExecuteMouseMoveTo(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string element_id;
  bool has_element = params.GetString("element", &element_id);
  int x_offset = 0;
  int y_offset = 0;
  bool has_offset = params.GetInteger("xoffset", &x_offset) &&
      params.GetInteger("yoffset", &y_offset);
  if (!has_element && !has_offset)
    return Status(kUnknownError, "at least an element or offset should be set");

  WebPoint location;
  if (has_element) {
    Status status = ScrollElementIntoView(
        session, web_view, element_id, &location);
    if (status.IsError())
      return status;
  } else {
    location = session->mouse_position;
  }

  if (has_offset) {
    location.Offset(x_offset, y_offset);
  } else {
    WebSize size;
    Status status = GetElementSize(session, web_view, element_id, &size);
    if (status.IsError())
      return status;
    location.Offset(size.width / 2, size.height / 2);
  }

  std::list<MouseEvent> events;
  events.push_back(
      MouseEvent(kMovedMouseEventType, kNoneMouseButton,
                 location.x, location.y, session->sticky_modifiers, 0));
  Status status =
      web_view->DispatchMouseEvents(events, session->GetCurrentFrameId());
  if (status.IsOk())
    session->mouse_position = location;
  return status;
}

Status ExecuteMouseClick(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  MouseButton button;
  Status status = GetMouseButton(params, &button);
  if (status.IsError())
    return status;
  std::list<MouseEvent> events;
  events.push_back(
      MouseEvent(kPressedMouseEventType, button,
                 session->mouse_position.x, session->mouse_position.y,
                 session->sticky_modifiers, 1));
  events.push_back(
      MouseEvent(kReleasedMouseEventType, button,
                 session->mouse_position.x, session->mouse_position.y,
                 session->sticky_modifiers, 1));
  return web_view->DispatchMouseEvents(events, session->GetCurrentFrameId());
}

Status ExecuteMouseButtonDown(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  MouseButton button;
  Status status = GetMouseButton(params, &button);
  if (status.IsError())
    return status;
  std::list<MouseEvent> events;
  events.push_back(
      MouseEvent(kPressedMouseEventType, button,
                 session->mouse_position.x, session->mouse_position.y,
                 session->sticky_modifiers, 1));
  return web_view->DispatchMouseEvents(events, session->GetCurrentFrameId());
}

Status ExecuteMouseButtonUp(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  MouseButton button;
  Status status = GetMouseButton(params, &button);
  if (status.IsError())
    return status;
  std::list<MouseEvent> events;
  events.push_back(
      MouseEvent(kReleasedMouseEventType, button,
                 session->mouse_position.x, session->mouse_position.y,
                 session->sticky_modifiers, 1));
  return web_view->DispatchMouseEvents(events, session->GetCurrentFrameId());
}

Status ExecuteMouseDoubleClick(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  MouseButton button;
  Status status = GetMouseButton(params, &button);
  if (status.IsError())
    return status;
  std::list<MouseEvent> events;
  events.push_back(
      MouseEvent(kPressedMouseEventType, button,
                 session->mouse_position.x, session->mouse_position.y,
                 session->sticky_modifiers, 2));
  events.push_back(
      MouseEvent(kReleasedMouseEventType, button,
                 session->mouse_position.x, session->mouse_position.y,
                 session->sticky_modifiers, 2));
  return web_view->DispatchMouseEvents(events, session->GetCurrentFrameId());
}

Status ExecuteTouchDown(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return ExecuteTouchEvent(session, web_view, kTouchStart, params);
}

Status ExecuteTouchUp(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return ExecuteTouchEvent(session, web_view, kTouchEnd, params);
}

Status ExecuteTouchMove(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return ExecuteTouchEvent(session, web_view, kTouchMove, params);
}

Status ExecuteGetActiveElement(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return GetActiveElement(session, web_view, value);
}

Status ExecuteSendKeysToActiveElement(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const base::ListValue* key_list;
  if (!params.GetList("value", &key_list))
    return Status(kUnknownError, "'value' must be a list");
  return SendKeysOnWindow(
      web_view, key_list, false, &session->sticky_modifiers);
}

Status ExecuteGetAppCacheStatus(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->EvaluateScript(
      session->GetCurrentFrameId(),
      "applicationCache.status",
      value);
}

Status ExecuteIsBrowserOnline(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->EvaluateScript(
      session->GetCurrentFrameId(),
      "navigator.onLine",
      value);
}

Status ExecuteGetStorageItem(
    const char* storage,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string key;
  if (!params.GetString("key", &key))
    return Status(kUnknownError, "'key' must be a string");
  base::ListValue args;
  args.Append(new base::StringValue(key));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      base::StringPrintf("function(key) { return %s[key]; }", storage),
      args,
      value);
}

Status ExecuteGetStorageKeys(
    const char* storage,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const char script[] =
      "var keys = [];"
      "for (var key in %s) {"
      "  keys.push(key);"
      "}"
      "keys";
  return web_view->EvaluateScript(
      session->GetCurrentFrameId(),
      base::StringPrintf(script, storage),
      value);
}

Status ExecuteSetStorageItem(
    const char* storage,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string key;
  if (!params.GetString("key", &key))
    return Status(kUnknownError, "'key' must be a string");
  std::string storage_value;
  if (!params.GetString("value", &storage_value))
    return Status(kUnknownError, "'value' must be a string");
  base::ListValue args;
  args.Append(new base::StringValue(key));
  args.Append(new base::StringValue(storage_value));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      base::StringPrintf("function(key, value) { %s[key] = value; }", storage),
      args,
      value);
}

Status ExecuteRemoveStorageItem(
    const char* storage,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string key;
  if (!params.GetString("key", &key))
    return Status(kUnknownError, "'key' must be a string");
  base::ListValue args;
  args.Append(new base::StringValue(key));
  return web_view->CallFunction(
      session->GetCurrentFrameId(),
      base::StringPrintf("function(key) { %s.removeItem(key) }", storage),
      args,
      value);
}

Status ExecuteClearStorage(
    const char* storage,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->EvaluateScript(
      session->GetCurrentFrameId(),
      base::StringPrintf("%s.clear()", storage),
      value);
}

Status ExecuteGetStorageSize(
    const char* storage,
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->EvaluateScript(
      session->GetCurrentFrameId(),
      base::StringPrintf("%s.length", storage),
      value);
}

Status ExecuteScreenshot(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  Status status = session->chrome->ActivateWebView(web_view->GetId());
  if (status.IsError())
    return status;

  // TODO(Peter Wang): need to implement xwalk extension to support it.

  return Status(kOk);
}

Status ExecuteGetCookies(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::list<Cookie> cookies;
  Status status = GetVisibleCookies(web_view, &cookies);
  if (status.IsError())
    return status;
  scoped_ptr<base::ListValue> cookie_list(new base::ListValue());
  for (std::list<Cookie>::const_iterator it = cookies.begin();
       it != cookies.end(); ++it) {
    cookie_list->Append(CreateDictionaryFrom(*it));
  }
  value->reset(cookie_list.release());
  return Status(kOk);
}

Status ExecuteAddCookie(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const base::DictionaryValue* cookie;
  if (!params.GetDictionary("cookie", &cookie))
    return Status(kUnknownError, "missing 'cookie'");
  base::ListValue args;
  args.Append(cookie->DeepCopy());
  scoped_ptr<base::Value> result;
  return web_view->CallFunction(
      session->GetCurrentFrameId(), kAddCookieScript, args, &result);
}

Status ExecuteDeleteCookie(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string name;
  if (!params.GetString("name", &name))
    return Status(kUnknownError, "missing 'name'");
  base::DictionaryValue params_url;
  scoped_ptr<base::Value> value_url;
  std::string url;
  Status status = GetUrl(web_view, session->GetCurrentFrameId(), &url);
  if (status.IsError())
    return status;
  return web_view->DeleteCookie(name, url);
}

Status ExecuteDeleteAllCookies(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::list<Cookie> cookies;
  Status status = GetVisibleCookies(web_view, &cookies);
  if (status.IsError())
    return status;

  if (!cookies.empty()) {
    base::DictionaryValue params_url;
    scoped_ptr<base::Value> value_url;
    std::string url;
    status = GetUrl(web_view, session->GetCurrentFrameId(), &url);
    if (status.IsError())
      return status;
    for (std::list<Cookie>::const_iterator it = cookies.begin();
         it != cookies.end(); ++it) {
      status = web_view->DeleteCookie(it->name, url);
      if (status.IsError())
        return status;
    }
  }

  return Status(kOk);
}

Status ExecuteSetLocation(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  const base::DictionaryValue* location = NULL;
  Geoposition geoposition;
  if (!params.GetDictionary("location", &location) ||
      !location->GetDouble("latitude", &geoposition.latitude) ||
      !location->GetDouble("longitude", &geoposition.longitude))
    return Status(kUnknownError, "missing or invalid 'location'");
  if (location->HasKey("accuracy") &&
      !location->GetDouble("accuracy", &geoposition.accuracy)) {
    return Status(kUnknownError, "invalid 'accuracy'");
  } else {
    // |accuracy| is not part of the WebDriver spec yet, so if it is not given
    // default to 100 meters accuracy.
    geoposition.accuracy = 100;
  }

  Status status = web_view->OverrideGeolocation(geoposition);
  if (status.IsOk())
    session->overridden_geoposition.reset(new Geoposition(geoposition));
  return status;
}

Status ExecuteTakeHeapSnapshot(
    Session* session,
    WebView* web_view,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  return web_view->TakeHeapSnapshot(value);
}
