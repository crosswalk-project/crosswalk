// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_DEVTOOLS_HTTP_CLIENT_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_DEVTOOLS_HTTP_CLIENT_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/xwalkdriver/net/sync_websocket_factory.h"

namespace base {
class TimeDelta;
}

class DevToolsClient;
class NetAddress;
class Status;
class URLRequestContextGetter;

struct WebViewInfo {
  enum Type {
    kApp,
    kBackgroundPage,
    kPage,
    kWorker,
    kOther
  };

  WebViewInfo(const std::string& id,
              const std::string& debugger_url,
              const std::string& url,
              Type type);
  ~WebViewInfo();

  bool IsFrontend() const;

  std::string id;
  std::string debugger_url;
  std::string url;
  Type type;
};

class WebViewsInfo {
 public:
  WebViewsInfo();
  explicit WebViewsInfo(const std::vector<WebViewInfo>& info);
  ~WebViewsInfo();

  const WebViewInfo& Get(int index) const;
  size_t GetSize() const;
  const WebViewInfo* GetForId(const std::string& id) const;

 private:
  std::vector<WebViewInfo> views_info;
};

class DevToolsHttpClient {
 public:
  DevToolsHttpClient(
      const NetAddress& address,
      scoped_refptr<URLRequestContextGetter> context_getter,
      const SyncWebSocketFactory& socket_factory);
  ~DevToolsHttpClient();

  Status Init(const base::TimeDelta& timeout);

  Status GetWebViewsInfo(WebViewsInfo* views_info);

  scoped_ptr<DevToolsClient> CreateClient(const std::string& id);

  Status CloseWebView(const std::string& id);

  Status ActivateWebView(const std::string& id);

  const std::string& version() const;
  int build_no() const;

 private:
  Status GetVersion(std::string* version);
  Status CloseFrontends(const std::string& for_client_id);
  bool FetchUrlAndLog(const std::string& url,
                      URLRequestContextGetter* getter,
                      std::string* response);

  scoped_refptr<URLRequestContextGetter> context_getter_;
  SyncWebSocketFactory socket_factory_;
  std::string server_url_;
  std::string web_socket_url_prefix_;
  std::string version_;
  int build_no_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsHttpClient);
};

namespace internal {
Status ParseWebViewsInfo(const std::string& data,
                         WebViewsInfo* views_info);
Status ParseVersionInfo(const std::string& data,
                        std::string* version);
}  // namespace internal

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_DEVTOOLS_HTTP_CLIENT_H_
