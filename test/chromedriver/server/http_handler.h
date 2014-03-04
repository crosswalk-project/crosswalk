// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_SERVER_HTTP_HANDLER_H_
#define CHROME_TEST_CHROMEDRIVER_SERVER_HTTP_HANDLER_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "chrome/test/chromedriver/command.h"
#include "chrome/test/chromedriver/commands.h"
#include "chrome/test/chromedriver/element_commands.h"
#include "chrome/test/chromedriver/net/sync_websocket_factory.h"
#include "chrome/test/chromedriver/session_commands.h"
#include "chrome/test/chromedriver/session_thread_map.h"
#include "chrome/test/chromedriver/window_commands.h"

namespace base {
class DictionaryValue;
class SingleThreadTaskRunner;
}

namespace net {
class HttpServerRequestInfo;
class HttpServerResponseInfo;
}

class Adb;
class DeviceManager;
class PortManager;
class PortServer;
class URLRequestContextGetter;

enum HttpMethod {
  kGet,
  kPost,
  kDelete,
};

struct CommandMapping {
  CommandMapping(HttpMethod method,
                 const std::string& path_pattern,
                 const Command& command);
  ~CommandMapping();

  HttpMethod method;
  std::string path_pattern;
  Command command;
};

typedef base::Callback<void(scoped_ptr<net::HttpServerResponseInfo>)>
    HttpResponseSenderFunc;

class HttpHandler {
 public:
  explicit HttpHandler(const std::string& url_base);
  HttpHandler(const base::Closure& quit_func,
              const scoped_refptr<base::SingleThreadTaskRunner> io_task_runner,
              const std::string& url_base,
              int adb_port,
              scoped_ptr<PortServer> port_server);
  ~HttpHandler();

  void Handle(const net::HttpServerRequestInfo& request,
              const HttpResponseSenderFunc& send_response_func);

 private:
  FRIEND_TEST_ALL_PREFIXES(HttpHandlerTest, HandleUnknownCommand);
  FRIEND_TEST_ALL_PREFIXES(HttpHandlerTest, HandleNewSession);
  FRIEND_TEST_ALL_PREFIXES(HttpHandlerTest, HandleInvalidPost);
  FRIEND_TEST_ALL_PREFIXES(HttpHandlerTest, HandleUnimplementedCommand);
  FRIEND_TEST_ALL_PREFIXES(HttpHandlerTest, HandleCommand);
  typedef std::vector<CommandMapping> CommandMap;

  Command WrapToCommand(const char* name,
                        const SessionCommand& session_command);
  Command WrapToCommand(const char* name, const WindowCommand& window_command);
  Command WrapToCommand(const char* name,
                        const ElementCommand& element_command);
  void HandleCommand(const net::HttpServerRequestInfo& request,
                     const std::string& trimmed_path,
                     const HttpResponseSenderFunc& send_response_func);
  void PrepareResponse(const std::string& trimmed_path,
                       const HttpResponseSenderFunc& send_response_func,
                       const Status& status,
                       scoped_ptr<base::Value> value,
                       const std::string& session_id);
  scoped_ptr<net::HttpServerResponseInfo> PrepareResponseHelper(
      const std::string& trimmed_path,
      const Status& status,
      scoped_ptr<base::Value> value,
      const std::string& session_id);

  base::ThreadChecker thread_checker_;
  base::Closure quit_func_;
  std::string url_base_;
  bool received_shutdown_;
  scoped_refptr<URLRequestContextGetter> context_getter_;
  SyncWebSocketFactory socket_factory_;
  SessionThreadMap session_thread_map_;
  scoped_ptr<CommandMap> command_map_;
  scoped_ptr<Adb> adb_;
  scoped_ptr<DeviceManager> device_manager_;
  scoped_ptr<PortServer> port_server_;
  scoped_ptr<PortManager> port_manager_;

  base::WeakPtrFactory<HttpHandler> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(HttpHandler);
};

namespace internal {

extern const char kNewSessionPathPattern[];

bool MatchesCommand(const std::string& method,
                    const std::string& path,
                    const CommandMapping& command,
                    std::string* session_id,
                    base::DictionaryValue* out_params);

}  // namespace internal

#endif  // CHROME_TEST_CHROMEDRIVER_SERVER_HTTP_HANDLER_H_
