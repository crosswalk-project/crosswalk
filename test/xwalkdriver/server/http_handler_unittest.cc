// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/bind.h"
#include "base/json/json_writer.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/log.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/command.h"
#include "xwalk/test/xwalkdriver/server/http_handler.h"
#include "net/http/http_status_code.h"
#include "net/server/http_server_request_info.h"
#include "net/server/http_server_response_info.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

void DummyCommand(
    const Status& status,
    const base::DictionaryValue& params,
    const std::string& session_id,
    const CommandCallback& callback) {
  callback.Run(status,
               scoped_ptr<base::Value>(new base::FundamentalValue(1)),
               "session_id");
}

void OnResponse(net::HttpServerResponseInfo* response_to_set,
                scoped_ptr<net::HttpServerResponseInfo> response) {
  *response_to_set = *response;
}

}  // namespace

TEST(HttpHandlerTest, HandleOutsideOfBaseUrl) {
  HttpHandler handler("base/url/");
  net::HttpServerRequestInfo request;
  request.method = "get";
  request.path = "base/path";
  request.data = "body";
  net::HttpServerResponseInfo response;
  handler.Handle(request, base::Bind(&OnResponse, &response));
  ASSERT_EQ(net::HTTP_BAD_REQUEST, response.status_code());
}

TEST(HttpHandlerTest, HandleUnknownCommand) {
  HttpHandler handler("/");
  net::HttpServerRequestInfo request;
  request.method = "get";
  request.path = "/path";
  net::HttpServerResponseInfo response;
  handler.Handle(request, base::Bind(&OnResponse, &response));
  ASSERT_EQ(net::HTTP_NOT_FOUND, response.status_code());
}

TEST(HttpHandlerTest, HandleNewSession) {
  HttpHandler handler("/base/");
  handler.command_map_.reset(new HttpHandler::CommandMap());
  handler.command_map_->push_back(
      CommandMapping(kPost, internal::kNewSessionPathPattern,
                     base::Bind(&DummyCommand, Status(kOk))));
  net::HttpServerRequestInfo request;
  request.method = "post";
  request.path = "/base/session";
  net::HttpServerResponseInfo response;
  handler.Handle(request, base::Bind(&OnResponse, &response));
  ASSERT_EQ(net::HTTP_SEE_OTHER, response.status_code());
  ASSERT_NE(std::string::npos,
            response.Serialize().find("Location:/base/session/"))
      << response.Serialize();
}

TEST(HttpHandlerTest, HandleInvalidPost) {
  HttpHandler handler("/");
  handler.command_map_->push_back(
      CommandMapping(kPost, "path", base::Bind(&DummyCommand, Status(kOk))));
  net::HttpServerRequestInfo request;
  request.method = "post";
  request.path = "/path";
  request.data = "should be a dictionary";
  net::HttpServerResponseInfo response;
  handler.Handle(request, base::Bind(&OnResponse, &response));
  ASSERT_EQ(net::HTTP_BAD_REQUEST, response.status_code());
}

TEST(HttpHandlerTest, HandleUnimplementedCommand) {
  HttpHandler handler("/");
  handler.command_map_->push_back(
      CommandMapping(kPost, "path",
                     base::Bind(&DummyCommand, Status(kUnknownCommand))));
  net::HttpServerRequestInfo request;
  request.method = "post";
  request.path = "/path";
  net::HttpServerResponseInfo response;
  handler.Handle(request, base::Bind(&OnResponse, &response));
  ASSERT_EQ(net::HTTP_NOT_IMPLEMENTED, response.status_code());
}

TEST(HttpHandlerTest, HandleCommand) {
  HttpHandler handler("/");
  handler.command_map_->push_back(
      CommandMapping(kPost, "path", base::Bind(&DummyCommand, Status(kOk))));
  net::HttpServerRequestInfo request;
  request.method = "post";
  request.path = "/path";
  net::HttpServerResponseInfo response;
  handler.Handle(request, base::Bind(&OnResponse, &response));
  ASSERT_EQ(net::HTTP_OK, response.status_code());
  base::DictionaryValue body;
  body.SetInteger("status", kOk);
  body.SetInteger("value", 1);
  body.SetString("sessionId", "session_id");
  std::string json;
  base::JSONWriter::Write(&body, &json);
  ASSERT_STREQ(json.c_str(), response.body().c_str());
}

TEST(MatchesCommandTest, DiffMethod) {
  CommandMapping command(kPost, "path", base::Bind(&DummyCommand, Status(kOk)));
  std::string session_id;
  base::DictionaryValue params;
  ASSERT_FALSE(internal::MatchesCommand(
      "get", "path", command, &session_id, &params));
  ASSERT_STREQ("", session_id.c_str());
  ASSERT_EQ(0u, params.size());
}

TEST(MatchesCommandTest, DiffPathLength) {
  CommandMapping command(kPost, "path/path",
                         base::Bind(&DummyCommand, Status(kOk)));
  std::string session_id;
  base::DictionaryValue params;
  ASSERT_FALSE(internal::MatchesCommand(
      "post", "path", command, &session_id, &params));
  ASSERT_FALSE(internal::MatchesCommand(
      "post", std::string(), command, &session_id, &params));
  ASSERT_FALSE(
      internal::MatchesCommand("post", "/", command, &session_id, &params));
  ASSERT_FALSE(internal::MatchesCommand(
      "post", "path/path/path", command, &session_id, &params));
}

TEST(MatchesCommandTest, DiffPaths) {
  CommandMapping command(kPost, "path/apath",
                         base::Bind(&DummyCommand, Status(kOk)));
  std::string session_id;
  base::DictionaryValue params;
  ASSERT_FALSE(internal::MatchesCommand(
      "post", "path/bpath", command, &session_id, &params));
}

TEST(MatchesCommandTest, Substitution) {
  CommandMapping command(kPost, "path/:sessionId/space/:a/:b",
                         base::Bind(&DummyCommand, Status(kOk)));
  std::string session_id;
  base::DictionaryValue params;
  ASSERT_TRUE(internal::MatchesCommand(
      "post", "path/1/space/2/3", command, &session_id, &params));
  ASSERT_EQ("1", session_id);
  ASSERT_EQ(2u, params.size());
  std::string param;
  ASSERT_TRUE(params.GetString("a", &param));
  ASSERT_EQ("2", param);
  ASSERT_TRUE(params.GetString("b", &param));
  ASSERT_EQ("3", param);
}
