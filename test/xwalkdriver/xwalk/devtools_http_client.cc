// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/devtools_http_client.h"

#include <list>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/json/json_reader.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/devtools_client_impl.h"
#include "xwalk/test/xwalkdriver/xwalk/log.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/xwalk/version.h"
#include "xwalk/test/xwalkdriver/xwalk/web_view_impl.h"
#include "xwalk/test/xwalkdriver/net/net_util.h"
#include "xwalk/test/xwalkdriver/net/url_request_context_getter.h"

namespace {

Status FakeCloseFrontends() {
  return Status(kOk);
}

}  // namespace

WebViewInfo::WebViewInfo(const std::string& id,
                         const std::string& debugger_url,
                         const std::string& url,
                         Type type)
    : id(id), debugger_url(debugger_url), url(url), type(type) {}

WebViewInfo::~WebViewInfo() {}

bool WebViewInfo::IsFrontend() const {
  return url.find("chrome-devtools://") == 0u;
}

WebViewsInfo::WebViewsInfo() {}

WebViewsInfo::WebViewsInfo(const std::vector<WebViewInfo>& info)
    : views_info(info) {}

WebViewsInfo::~WebViewsInfo() {}

const WebViewInfo& WebViewsInfo::Get(int index) const {
  return views_info[index];
}

size_t WebViewsInfo::GetSize() const {
  return views_info.size();
}

const WebViewInfo* WebViewsInfo::GetForId(const std::string& id) const {
  for (size_t i = 0; i < views_info.size(); ++i) {
    if (views_info[i].id == id)
      return &views_info[i];
  }
  return NULL;
}

DevToolsHttpClient::DevToolsHttpClient(
    const NetAddress& address,
    scoped_refptr<URLRequestContextGetter> context_getter,
    const SyncWebSocketFactory& socket_factory)
    : context_getter_(context_getter),
      socket_factory_(socket_factory),
      server_url_("http://" + address.ToString()),
      web_socket_url_prefix_(base::StringPrintf(
          "ws://%s/devtools/page/", address.ToString().c_str())) {}

DevToolsHttpClient::~DevToolsHttpClient() {}

Status DevToolsHttpClient::Init(const base::TimeDelta& timeout) {
  base::TimeTicks deadline = base::TimeTicks::Now() + timeout;
  std::string devtools_version;
  while (true) {
    Status status = GetVersion(&devtools_version);
    if (status.IsOk())
      break;
    if (status.code() != kXwalkNotReachable ||
        base::TimeTicks::Now() > deadline) {
      return status;
    }
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(50));
  }

  int kToTBuildNo = 9999;
  if (devtools_version.empty()) {
    // Content Shell has an empty product version and a fake user agent.
    // There's no way to detect the actual version, so assume it is tip of tree.
    version_ = "content shell";
    build_no_ = kToTBuildNo;
    return Status(kOk);
  }
  if (devtools_version.find("Version/") == 0u) {
    version_ = "webview";
    build_no_ = kToTBuildNo;
    return Status(kOk);
  }
  std::string prefix = "Xwalk/";
  if (devtools_version.find(prefix) != 0u) {
    return Status(kUnknownError,
                  "unrecognized Xwalk version: " + devtools_version);
  }

  std::string stripped_version = devtools_version.substr(prefix.length());
  int temp_build_no;
  std::vector<std::string> version_parts;
  base::SplitString(stripped_version, '.', &version_parts);
  if (version_parts.size() != 4 ||
      !base::StringToInt(version_parts[2], &temp_build_no)) {
    return Status(kUnknownError,
                  "unrecognized Xwalk version: " + devtools_version);
  }

  version_ = stripped_version;
  build_no_ = temp_build_no;
  return Status(kOk);
}

Status DevToolsHttpClient::GetWebViewsInfo(WebViewsInfo* views_info) {
  std::string data;
  if (!FetchUrlAndLog(server_url_ + "/json", context_getter_.get(), &data))
    return Status(kXwalkNotReachable);

  return internal::ParseWebViewsInfo(data, views_info);
}

scoped_ptr<DevToolsClient> DevToolsHttpClient::CreateClient(
    const std::string& id) {
  return scoped_ptr<DevToolsClient>(new DevToolsClientImpl(
      socket_factory_,
      web_socket_url_prefix_ + id,
      id,
      base::Bind(
          &DevToolsHttpClient::CloseFrontends, base::Unretained(this), id)));
}

Status DevToolsHttpClient::CloseWebView(const std::string& id) {
  std::string data;
  if (!FetchUrlAndLog(
          server_url_ + "/json/close/" + id, context_getter_.get(), &data)) {
    return Status(kOk);  // Closing the last web view leads xwalk to quit.
  }

  // Wait for the target window to be completely closed.
  base::TimeTicks deadline =
      base::TimeTicks::Now() + base::TimeDelta::FromSeconds(20);
  while (base::TimeTicks::Now() < deadline) {
    WebViewsInfo views_info;
    Status status = GetWebViewsInfo(&views_info);
    if (status.code() == kXwalkNotReachable)
      return Status(kOk);
    if (status.IsError())
      return status;
    if (!views_info.GetForId(id))
      return Status(kOk);
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(50));
  }
  return Status(kUnknownError, "failed to close window in 20 seconds");
}

Status DevToolsHttpClient::ActivateWebView(const std::string& id) {
  std::string data;
  if (!FetchUrlAndLog(
          server_url_ + "/json/activate/" + id, context_getter_.get(), &data))
    return Status(kUnknownError, "cannot activate web view");
  return Status(kOk);
}

const std::string& DevToolsHttpClient::version() const {
  return version_;
}

int DevToolsHttpClient::build_no() const {
  return build_no_;
}

Status DevToolsHttpClient::GetVersion(std::string* version) {
  std::string data;
  if (!FetchUrlAndLog(
          server_url_ + "/json/version", context_getter_.get(), &data))
    return Status(kXwalkNotReachable);

  return internal::ParseVersionInfo(data, version);
}

Status DevToolsHttpClient::CloseFrontends(const std::string& for_client_id) {
  WebViewsInfo views_info;
  Status status = GetWebViewsInfo(&views_info);
  if (status.IsError())
    return status;

  // Close frontends. Usually frontends are docked in the same page, although
  // some may be in tabs (undocked, xwalk://inspect, the DevTools
  // discovery page, etc.). Tabs can be closed via the DevTools HTTP close
  // URL, but docked frontends can only be closed, by design, by connecting
  // to them and clicking the close button. Close the tab frontends first
  // in case one of them is debugging a docked frontend, which would prevent
  // the code from being able to connect to the docked one.
  std::list<std::string> tab_frontend_ids;
  std::list<std::string> docked_frontend_ids;
  for (size_t i = 0; i < views_info.GetSize(); ++i) {
    const WebViewInfo& view_info = views_info.Get(i);
    if (view_info.IsFrontend()) {
      if (view_info.type == WebViewInfo::kPage)
        tab_frontend_ids.push_back(view_info.id);
      else if (view_info.type == WebViewInfo::kOther)
        docked_frontend_ids.push_back(view_info.id);
      else
        return Status(kUnknownError, "unknown type of DevTools frontend");
    }
  }

  for (std::list<std::string>::const_iterator it = tab_frontend_ids.begin();
       it != tab_frontend_ids.end(); ++it) {
    status = CloseWebView(*it);
    if (status.IsError())
      return status;
  }

  for (std::list<std::string>::const_iterator it = docked_frontend_ids.begin();
       it != docked_frontend_ids.end(); ++it) {
    scoped_ptr<DevToolsClient> client(new DevToolsClientImpl(
        socket_factory_,
        web_socket_url_prefix_ + *it,
        *it,
        base::Bind(&FakeCloseFrontends)));
    scoped_ptr<WebViewImpl> web_view(
        new WebViewImpl(*it, build_no_, client.Pass()));

    status = web_view->ConnectIfNecessary();
    // Ignore disconnected error, because the debugger might have closed when
    // its container page was closed above.
    if (status.IsError() && status.code() != kDisconnected)
      return status;

    scoped_ptr<base::Value> result;
    status = web_view->EvaluateScript(
        std::string(),
        "document.querySelector('*[id^=\"close-button-\"]').click();",
        &result);
    // Ignore disconnected error, because it may be closed already.
    if (status.IsError() && status.code() != kDisconnected)
      return status;
  }

  // Wait until DevTools UI disconnects from the given web view.
  base::TimeTicks deadline =
      base::TimeTicks::Now() + base::TimeDelta::FromSeconds(20);
  while (base::TimeTicks::Now() < deadline) {
    status = GetWebViewsInfo(&views_info);
    if (status.IsError())
      return status;

    const WebViewInfo* view_info = views_info.GetForId(for_client_id);
    if (!view_info)
      return Status(kNoSuchWindow, "window was already closed");
    if (view_info->debugger_url.size())
      return Status(kOk);

    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(50));
  }
  return Status(kUnknownError, "failed to close UI debuggers");
}

bool DevToolsHttpClient::FetchUrlAndLog(const std::string& url,
                                        URLRequestContextGetter* getter,
                                        std::string* response) {
  VLOG(1) << "DevTools request: " << url;
  bool ok = FetchUrl(url, getter, response);
  if (ok) {
    VLOG(1) << "DevTools response: " << *response;
  } else {
    VLOG(1) << "DevTools request failed";
  }
  return ok;
}

namespace internal {

Status ParseWebViewsInfo(const std::string& data,
                         WebViewsInfo* views_info) {
  scoped_ptr<base::Value> value(base::JSONReader::Read(data));
  if (!value.get())
    return Status(kUnknownError, "DevTools returned invalid JSON");
  base::ListValue* list;
  if (!value->GetAsList(&list))
    return Status(kUnknownError, "DevTools did not return list");

  std::vector<WebViewInfo> temp_views_info;
  for (size_t i = 0; i < list->GetSize(); ++i) {
    base::DictionaryValue* info;
    if (!list->GetDictionary(i, &info))
      return Status(kUnknownError, "DevTools contains non-dictionary item");
    std::string id;
    if (!info->GetString("id", &id))
      return Status(kUnknownError, "DevTools did not include id");
    std::string type_as_string;
    if (!info->GetString("type", &type_as_string))
      return Status(kUnknownError, "DevTools did not include type");
    std::string url;
    if (!info->GetString("url", &url))
      return Status(kUnknownError, "DevTools did not include url");
    std::string debugger_url;
    info->GetString("webSocketDebuggerUrl", &debugger_url);
    WebViewInfo::Type type;
    if (type_as_string == "app")
      type = WebViewInfo::kApp;
    else if (type_as_string == "background_page")
      type = WebViewInfo::kBackgroundPage;
    else if (type_as_string == "page")
      type = WebViewInfo::kPage;
    else if (type_as_string == "worker")
      type = WebViewInfo::kWorker;
    else if (type_as_string == "other")
      type = WebViewInfo::kOther;
    else
      return Status(kUnknownError,
                    "DevTools returned unknown type:" + type_as_string);
    temp_views_info.push_back(WebViewInfo(id, debugger_url, url, type));
  }
  *views_info = WebViewsInfo(temp_views_info);
  return Status(kOk);
}

Status ParseVersionInfo(const std::string& data,
                        std::string* version) {
  scoped_ptr<base::Value> value(base::JSONReader::Read(data));
  if (!value.get())
    return Status(kUnknownError, "version info not in JSON");
  base::DictionaryValue* dict;
  if (!value->GetAsDictionary(&dict))
    return Status(kUnknownError, "version info not a dictionary");
  if (!dict->GetString("Browser", version)) {
    return Status(
        kUnknownError,
        "Xwalk version must be >= " + GetMinimumSupportedXwalkVersion(),
        Status(kUnknownError, "version info doesn't include string 'Browser'"));
  }
  return Status(kOk);
}

}  // namespace internal
