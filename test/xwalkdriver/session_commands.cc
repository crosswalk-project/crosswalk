// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/session_commands.h"

#include <list>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/file_util.h"
#include "base/logging.h"  // For CHECK macros.
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/basic_types.h"
#include "xwalk/test/xwalkdriver/capabilities.h"
#include "xwalk/test/xwalkdriver/xwalk/xwalk.h"
#include "xwalk/test/xwalkdriver/xwalk/xwalk_android_impl.h"
#include "xwalk/test/xwalkdriver/xwalk/xwalk_desktop_impl.h"
#include "xwalk/test/xwalkdriver/xwalk/device_manager.h"
#include "xwalk/test/xwalkdriver/xwalk/devtools_event_listener.h"
#include "xwalk/test/xwalkdriver/xwalk/geoposition.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/xwalk/web_view.h"
#include "xwalk/test/xwalkdriver/xwalk_launcher.h"
#include "xwalk/test/xwalkdriver/logging.h"
#include "xwalk/test/xwalkdriver/net/url_request_context_getter.h"
#include "xwalk/test/xwalkdriver/session.h"
#include "xwalk/test/xwalkdriver/util.h"
#include "xwalk/test/xwalkdriver/version.h"

namespace {

const char kWindowHandlePrefix[] = "CDwindow-";

std::string WebViewIdToWindowHandle(const std::string& web_view_id) {
  return kWindowHandlePrefix + web_view_id;
}

bool WindowHandleToWebViewId(const std::string& window_handle,
                             std::string* web_view_id) {
  if (window_handle.find(kWindowHandlePrefix) != 0u)
    return false;
  *web_view_id = window_handle.substr(
      std::string(kWindowHandlePrefix).length());
  return true;
}

}  // namespace

InitSessionParams::InitSessionParams(
    scoped_refptr<URLRequestContextGetter> context_getter,
    const SyncWebSocketFactory& socket_factory,
    DeviceManager* device_manager,
    PortServer* port_server,
    PortManager* port_manager)
    : context_getter(context_getter),
      socket_factory(socket_factory),
      device_manager(device_manager),
      port_server(port_server),
      port_manager(port_manager) {}

InitSessionParams::~InitSessionParams() {}

namespace {

scoped_ptr<base::DictionaryValue> CreateCapabilities(Xwalk* xwalk) {
  scoped_ptr<base::DictionaryValue> caps(new base::DictionaryValue());
  caps->SetString("browserName", "xwalk");
  caps->SetString("version", xwalk->GetVersion());
  caps->SetString("xwalk.xwalkdriverVersion", kXwalkDriverVersion);
  caps->SetString("platform", xwalk->GetOperatingSystemName());
  caps->SetBoolean("javascriptEnabled", true);
  caps->SetBoolean("takesScreenshot", true);
  caps->SetBoolean("takesHeapSnapshot", true);
  caps->SetBoolean("handlesAlerts", true);
  caps->SetBoolean("databaseEnabled", false);
  caps->SetBoolean("locationContextEnabled", true);
  caps->SetBoolean("applicationCacheEnabled", false);
  caps->SetBoolean("browserConnectionEnabled", false);
  caps->SetBoolean("cssSelectorsEnabled", true);
  caps->SetBoolean("webStorageEnabled", true);
  caps->SetBoolean("rotatable", false);
  caps->SetBoolean("acceptSslCerts", true);
  caps->SetBoolean("nativeEvents", true);
  scoped_ptr<base::DictionaryValue> xwalk_caps(new base::DictionaryValue());
  if (xwalk->GetAsDesktop()) {
    xwalk_caps->SetString(
        "userDataDir",
        xwalk->GetAsDesktop()->command().GetSwitchValueNative(
            "user-data-dir"));
  }
  caps->Set("xwalk", xwalk_caps.release());
  return caps.Pass();
}


Status InitSessionHelper(
    const InitSessionParams& bound_params,
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  session->driver_log.reset(
      new WebDriverLog(WebDriverLog::kDriverType, Log::kAll));
  const base::DictionaryValue* desired_caps;
  if (!params.GetDictionary("desiredCapabilities", &desired_caps))
    return Status(kUnknownError, "cannot find dict 'desiredCapabilities'");

  Capabilities capabilities;
  Status status = capabilities.Parse(*desired_caps);
  if (status.IsError())
    return status;

  Log::Level driver_level = Log::kWarning;
  if (capabilities.logging_prefs.count(WebDriverLog::kDriverType))
    driver_level = capabilities.logging_prefs[WebDriverLog::kDriverType];
  session->driver_log->set_min_level(driver_level);

  // Create Log's and DevToolsEventListener's for ones that are DevTools-based.
  // Session will own the Log's, Xwalk will own the listeners.
  ScopedVector<DevToolsEventListener> devtools_event_listeners;
  status = CreateLogs(capabilities,
                      &session->devtools_logs,
                      &devtools_event_listeners);
  if (status.IsError())
    return status;

  status = LaunchXwalk(bound_params.context_getter.get(),
                        bound_params.socket_factory,
                        bound_params.device_manager,
                        bound_params.port_server,
                        bound_params.port_manager,
                        capabilities,
                        devtools_event_listeners,
                        &session->xwalk);
  if (status.IsError())
    return status;

  std::list<std::string> web_view_ids;
  status = session->xwalk->GetWebViewIds(&web_view_ids);
  if (status.IsError() || web_view_ids.empty()) {
    return status.IsError() ? status :
        Status(kUnknownError, "unable to discover open window in xwalk");
  }

  session->window = web_view_ids.front();
  session->detach = capabilities.detach;
  session->force_devtools_screenshot = capabilities.force_devtools_screenshot;
  session->capabilities = CreateCapabilities(session->xwalk.get());
  value->reset(session->capabilities->DeepCopy());
  return Status(kOk);
}

}  // namespace

Status ExecuteInitSession(
    const InitSessionParams& bound_params,
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  Status status = InitSessionHelper(bound_params, session, params, value);
  if (status.IsError())
    session->quit = true;
  return status;
}

Status ExecuteQuit(
    bool allow_detach,
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  session->quit = true;
  if (allow_detach && session->detach)
    return Status(kOk);
  else
    return session->xwalk->Quit();
}

Status ExecuteGetSessionCapabilities(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  value->reset(session->capabilities->DeepCopy());
  return Status(kOk);
}

Status ExecuteGetCurrentWindowHandle(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  WebView* web_view = NULL;
  Status status = session->GetTargetWindow(&web_view);
  if (status.IsError())
    return status;

  value->reset(
      new base::StringValue(WebViewIdToWindowHandle(web_view->GetId())));
  return Status(kOk);
}

Status ExecuteClose(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::list<std::string> web_view_ids;
  Status status = session->xwalk->GetWebViewIds(&web_view_ids);
  if (status.IsError())
    return status;
  bool is_last_web_view = web_view_ids.size() == 1u;
  web_view_ids.clear();

  WebView* web_view = NULL;
  status = session->GetTargetWindow(&web_view);
  if (status.IsError())
    return status;

  status = session->xwalk->CloseWebView(web_view->GetId());
  if (status.IsError())
    return status;

  status = session->xwalk->GetWebViewIds(&web_view_ids);
  if ((status.code() == kXwalkNotReachable && is_last_web_view) ||
      (status.IsOk() && web_view_ids.empty())) {
    // If no window is open, close is the equivalent of calling "quit".
    session->quit = true;
    return session->xwalk->Quit();
  }

  return status;
}

Status ExecuteGetWindowHandles(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::list<std::string> web_view_ids;
  Status status = session->xwalk->GetWebViewIds(&web_view_ids);
  if (status.IsError())
    return status;
  scoped_ptr<base::ListValue> window_ids(new base::ListValue());
  for (std::list<std::string>::const_iterator it = web_view_ids.begin();
       it != web_view_ids.end(); ++it) {
    window_ids->AppendString(WebViewIdToWindowHandle(*it));
  }
  value->reset(window_ids.release());
  return Status(kOk);
}

Status ExecuteSwitchToWindow(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string name;
  if (!params.GetString("name", &name) || name.empty())
    return Status(kUnknownError, "'name' must be a nonempty string");

  std::list<std::string> web_view_ids;
  Status status = session->xwalk->GetWebViewIds(&web_view_ids);
  if (status.IsError())
    return status;

  std::string web_view_id;
  bool found = false;
  if (WindowHandleToWebViewId(name, &web_view_id)) {
    // Check if any web_view matches |web_view_id|.
    for (std::list<std::string>::const_iterator it = web_view_ids.begin();
         it != web_view_ids.end(); ++it) {
      if (*it == web_view_id) {
        found = true;
        break;
      }
    }
  } else {
    // Check if any of the tab window names match |name|.
    const char* kGetWindowNameScript = "function() { return window.name; }";
    base::ListValue args;
    for (std::list<std::string>::const_iterator it = web_view_ids.begin();
         it != web_view_ids.end(); ++it) {
      scoped_ptr<base::Value> result;
      WebView* web_view;
      status = session->xwalk->GetWebViewById(*it, &web_view);
      if (status.IsError())
        return status;
      status = web_view->ConnectIfNecessary();
      if (status.IsError())
        return status;
      status = web_view->CallFunction(
          std::string(), kGetWindowNameScript, args, &result);
      if (status.IsError())
        return status;
      std::string window_name;
      if (!result->GetAsString(&window_name))
        return Status(kUnknownError, "failed to get window name");
      if (window_name == name) {
        web_view_id = *it;
        found = true;
        break;
      }
    }
  }

  if (!found)
    return Status(kNoSuchWindow);

  if (session->overridden_geoposition) {
    WebView* web_view;
    status = session->xwalk->GetWebViewById(web_view_id, &web_view);
    if (status.IsError())
      return status;
    status = web_view->ConnectIfNecessary();
    if (status.IsError())
      return status;
    status = web_view->OverrideGeolocation(*session->overridden_geoposition);
    if (status.IsError())
      return status;
  }

  session->window = web_view_id;
  session->SwitchToTopFrame();
  session->mouse_position = WebPoint(0, 0);
  return Status(kOk);
}

Status ExecuteSetTimeout(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  double ms_double;
  if (!params.GetDouble("ms", &ms_double))
    return Status(kUnknownError, "'ms' must be a double");
  std::string type;
  if (!params.GetString("type", &type))
    return Status(kUnknownError, "'type' must be a string");

  base::TimeDelta timeout =
      base::TimeDelta::FromMilliseconds(static_cast<int>(ms_double));
  // TODO(frankf): implicit and script timeout should be cleared
  // if negative timeout is specified.
  if (type == "implicit") {
    session->implicit_wait = timeout;
  } else if (type == "script") {
    session->script_timeout = timeout;
  } else if (type == "page load") {
    session->page_load_timeout =
        ((timeout < base::TimeDelta()) ? Session::kDefaultPageLoadTimeout
                                       : timeout);
  } else {
    return Status(kUnknownError, "unknown type of timeout:" + type);
  }
  return Status(kOk);
}

Status ExecuteSetScriptTimeout(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  double ms;
  if (!params.GetDouble("ms", &ms) || ms < 0)
    return Status(kUnknownError, "'ms' must be a non-negative number");
  session->script_timeout =
      base::TimeDelta::FromMilliseconds(static_cast<int>(ms));
  return Status(kOk);
}

Status ExecuteImplicitlyWait(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  double ms;
  if (!params.GetDouble("ms", &ms) || ms < 0)
    return Status(kUnknownError, "'ms' must be a non-negative number");
  session->implicit_wait =
      base::TimeDelta::FromMilliseconds(static_cast<int>(ms));
  return Status(kOk);
}

Status ExecuteIsLoading(
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

  bool is_pending;
  status = web_view->IsPendingNavigation(
      session->GetCurrentFrameId(), &is_pending);
  if (status.IsError())
    return status;
  value->reset(new base::FundamentalValue(is_pending));
  return Status(kOk);
}

Status ExecuteGetLocation(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  if (!session->overridden_geoposition) {
    return Status(kUnknownError,
                  "Location must be set before it can be retrieved");
  }
  base::DictionaryValue location;
  location.SetDouble("latitude", session->overridden_geoposition->latitude);
  location.SetDouble("longitude", session->overridden_geoposition->longitude);
  location.SetDouble("accuracy", session->overridden_geoposition->accuracy);
  // Set a dummy altitude to make WebDriver clients happy.
  // https://code.google.com/p/chromedriver/issues/detail?id=281
  location.SetDouble("altitude", 0);
  value->reset(location.DeepCopy());
  return Status(kOk);
}

Status ExecuteGetWindowPosition(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  (void) session;
  (void) params;
  (void) value;

  // TODO(Peter Wang): Implement "get position" of xwalk window.

  return Status(kOk);
}

Status ExecuteSetWindowPosition(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  (void) session;
  (void) params;
  (void) value;

  // TODO(Peter Wang): Implement "set position" of xwalk window.

  return Status(kOk);
}

Status ExecuteGetWindowSize(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  (void) session;
  (void) params;
  (void) value;

  // TODO(Peter Wang): Implement "get size" of xwalk window.

  return Status(kOk);
}

Status ExecuteSetWindowSize(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  (void) session;
  (void) params;
  (void) value;

  // TODO(Peter Wang): Implement "set size" of xwalk window.

  return Status(kOk);
}

Status ExecuteMaximizeWindow(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  (void) session;
  (void) params;
  (void) value;

  // TODO(Peter Wang): Implement maximization of xwalk window.

  return Status(kOk);
}

Status ExecuteGetAvailableLogTypes(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  scoped_ptr<base::ListValue> types(new base::ListValue());
  std::vector<WebDriverLog*> logs = session->GetAllLogs();
  for (std::vector<WebDriverLog*>::const_iterator log = logs.begin();
       log != logs.end();
       ++log) {
    types->AppendString((*log)->type());
  }
  *value = types.Pass();
  return Status(kOk);
}

Status ExecuteGetLog(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  std::string log_type;
  if (!params.GetString("type", &log_type)) {
    return Status(kUnknownError, "missing or invalid 'type'");
  }
  std::vector<WebDriverLog*> logs = session->GetAllLogs();
  for (std::vector<WebDriverLog*>::const_iterator log = logs.begin();
       log != logs.end();
       ++log) {
    if (log_type == (*log)->type()) {
      *value = (*log)->GetAndClearEntries();
      return Status(kOk);
    }
  }
  return Status(kUnknownError, "log type '" + log_type + "' not found");
}

Status ExecuteUploadFile(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
    std::string base64_zip_data;
  if (!params.GetString("file", &base64_zip_data))
    return Status(kUnknownError, "missing or invalid 'file'");
  std::string zip_data;
  if (!Base64Decode(base64_zip_data, &zip_data))
    return Status(kUnknownError, "unable to decode 'file'");

  if (!session->temp_dir.IsValid()) {
    if (!session->temp_dir.CreateUniqueTempDir())
      return Status(kUnknownError, "unable to create temp dir");
  }
  base::FilePath upload_dir;
  if (!base::CreateTemporaryDirInDir(
          session->temp_dir.path(), FILE_PATH_LITERAL("upload"), &upload_dir)) {
    return Status(kUnknownError, "unable to create temp dir");
  }
  std::string error_msg;
  base::FilePath upload;
  Status status = UnzipSoleFile(upload_dir, zip_data, &upload);
  if (status.IsError())
    return Status(kUnknownError, "unable to unzip 'file'", status);

  value->reset(new base::StringValue(upload.value()));
  return Status(kOk);
}
