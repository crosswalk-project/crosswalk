// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/compiler_specific.h"
#include "base/values.h"
#include "chrome/test/chromedriver/chrome/devtools_http_client.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

void ExpectEqual(const WebViewInfo& info1,
                 const WebViewInfo& info2) {
  EXPECT_EQ(info1.id, info2.id);
  EXPECT_EQ(info1.type, info2.type);
  EXPECT_EQ(info1.url, info2.url);
  EXPECT_EQ(info1.debugger_url, info2.debugger_url);
}

}  // namespace

TEST(ParseWebViewsInfo, Normal) {
  WebViewsInfo views_info;
  Status status = internal::ParseWebViewsInfo(
      "[{\"type\": \"page\", \"id\": \"1\", \"url\": \"http://page1\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"}]",
      &views_info);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, views_info.GetSize());
  ExpectEqual(
      WebViewInfo(
          "1", "ws://debugurl1", "http://page1", WebViewInfo::kPage),
      *views_info.GetForId("1"));
}

TEST(ParseWebViewsInfo, Multiple) {
  WebViewsInfo views_info;
  Status status = internal::ParseWebViewsInfo(
      "[{\"type\": \"page\", \"id\": \"1\", \"url\": \"http://page1\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"},"
      " {\"type\": \"other\", \"id\": \"2\", \"url\": \"http://page2\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl2\"}]",
      &views_info);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(2u, views_info.GetSize());
  ExpectEqual(
      WebViewInfo(
          "1", "ws://debugurl1", "http://page1", WebViewInfo::kPage),
      views_info.Get(0));
  ExpectEqual(
      WebViewInfo(
          "2", "ws://debugurl2", "http://page2", WebViewInfo::kOther),
      views_info.Get(1));
}

TEST(ParseWebViewsInfo, WithoutDebuggerUrl) {
  WebViewsInfo views_info;
  Status status = internal::ParseWebViewsInfo(
      "[{\"type\": \"page\", \"id\": \"1\", \"url\": \"http://page1\"}]",
      &views_info);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, views_info.GetSize());
  ExpectEqual(
      WebViewInfo("1", std::string(), "http://page1", WebViewInfo::kPage),
      views_info.Get(0));
}

namespace {

void AssertTypeIsOk(const std::string& type_as_string, WebViewInfo::Type type) {
  WebViewsInfo views_info;
  std::string data = "[{\"type\": \"" + type_as_string
      + "\", \"id\": \"1\", \"url\": \"http://page1\"}]";
  Status status = internal::ParseWebViewsInfo(data, &views_info);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ(1u, views_info.GetSize());
  ExpectEqual(WebViewInfo("1", std::string(), "http://page1", type),
              views_info.Get(0));
}

void AssertFails(const std::string& data) {
  WebViewsInfo views_info;
  Status status = internal::ParseWebViewsInfo(data, &views_info);
  ASSERT_FALSE(status.IsOk());
  ASSERT_EQ(0u, views_info.GetSize());
}

}  // namespace

TEST(ParseWebViewsInfo, Types) {
  AssertTypeIsOk("app", WebViewInfo::kApp);
  AssertTypeIsOk("background_page", WebViewInfo::kBackgroundPage);
  AssertTypeIsOk("page", WebViewInfo::kPage);
  AssertTypeIsOk("worker", WebViewInfo::kWorker);
  AssertTypeIsOk("other", WebViewInfo::kOther);
  AssertFails("[{\"type\": \"\", \"id\": \"1\", \"url\": \"http://page1\"}]");
}

TEST(ParseWebViewsInfo, NonList) {
  AssertFails("{\"id\": \"1\"}");
}

TEST(ParseWebViewsInfo, NonDictionary) {
  AssertFails("[1]");
}

TEST(ParseWebViewsInfo, NoId) {
  AssertFails(
      "[{\"type\": \"page\", \"url\": \"http://page1\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"}]");
}

TEST(ParseWebViewsInfo, InvalidId) {
  AssertFails(
      "[{\"type\": \"page\", \"id\": 1, \"url\": \"http://page1\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"}]");
}

TEST(ParseWebViewsInfo, NoType) {
  AssertFails(
      "[{\"id\": \"1\", \"url\": \"http://page1\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"}]");
}

TEST(ParseWebViewsInfo, InvalidType) {
  AssertFails(
      "[{\"type\": \"123\", \"id\": \"1\", \"url\": \"http://page1\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"}]");
}

TEST(ParseWebViewsInfo, NoUrl) {
  AssertFails(
      "[{\"type\": \"page\", \"id\": \"1\","
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"}]");
}

TEST(ParseWebViewsInfo, InvalidUrl) {
  AssertFails(
      "[{\"type\": \"page\", \"id\": \"1\", \"url\": 1,"
      "  \"webSocketDebuggerUrl\": \"ws://debugurl1\"}]");
}

namespace {

void AssertVersionFails(const std::string& data) {
  std::string version;
  Status status = internal::ParseVersionInfo(data, &version);
  ASSERT_TRUE(status.IsError());
  ASSERT_TRUE(version.empty());
}

}  // namespace

TEST(ParseVersionInfo, InvalidJSON) {
  AssertVersionFails("[");
}

TEST(ParseVersionInfo, NonDict) {
  AssertVersionFails("[]");
}

TEST(ParseVersionInfo, NoBrowserKey) {
  AssertVersionFails("{}");
}

TEST(ParseVersionInfo, Valid) {
  std::string version;
  Status status = internal::ParseVersionInfo("{\"Browser\": \"1\"}", &version);
  ASSERT_TRUE(status.IsOk());
  ASSERT_EQ("1", version);
}

