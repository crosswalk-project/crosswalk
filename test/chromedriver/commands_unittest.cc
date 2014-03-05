// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/stub_chrome.h"
#include "xwalk/test/chromedriver/chrome/stub_web_view.h"
#include "xwalk/test/chromedriver/chrome/web_view.h"
#include "xwalk/test/chromedriver/commands.h"
#include "xwalk/test/chromedriver/element_commands.h"
#include "xwalk/test/chromedriver/session.h"
#include "xwalk/test/chromedriver/session_commands.h"
#include "xwalk/test/chromedriver/window_commands.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/webdriver/atoms.h"

namespace {

void OnGetStatus(const Status& status,
                 scoped_ptr<base::Value> value,
                 const std::string& session_id) {
  ASSERT_EQ(kOk, status.code());
  base::DictionaryValue* dict;
  ASSERT_TRUE(value->GetAsDictionary(&dict));
  base::Value* unused;
  ASSERT_TRUE(dict->Get("os.name", &unused));
  ASSERT_TRUE(dict->Get("os.version", &unused));
  ASSERT_TRUE(dict->Get("os.arch", &unused));
  ASSERT_TRUE(dict->Get("build.version", &unused));
}

}  // namespace

TEST(CommandsTest, GetStatus) {
  base::DictionaryValue params;
  ExecuteGetStatus(params, std::string(), base::Bind(&OnGetStatus));
}

namespace {

void ExecuteStubQuit(
    int* count,
    const base::DictionaryValue& params,
    const std::string& session_id,
    const CommandCallback& callback) {
  if (*count == 0) {
    EXPECT_STREQ("id", session_id.c_str());
  } else {
    EXPECT_STREQ("id2", session_id.c_str());
  }
  (*count)++;
  callback.Run(Status(kOk), scoped_ptr<base::Value>(), session_id);
}

void OnQuitAll(const Status& status,
               scoped_ptr<base::Value> value,
               const std::string& session_id) {
  ASSERT_EQ(kOk, status.code());
  ASSERT_FALSE(value.get());
}

}  // namespace

TEST(CommandsTest, QuitAll) {
  SessionThreadMap map;
  Session session("id");
  Session session2("id2");
  map[session.id] = make_linked_ptr(new base::Thread("1"));
  map[session2.id] = make_linked_ptr(new base::Thread("2"));

  int count = 0;
  Command cmd = base::Bind(&ExecuteStubQuit, &count);
  base::DictionaryValue params;
  base::MessageLoop loop;
  ExecuteQuitAll(cmd, &map, params, std::string(), base::Bind(&OnQuitAll));
  ASSERT_EQ(2, count);
}

namespace {

Status ExecuteSimpleCommand(
    const std::string& expected_id,
    base::DictionaryValue* expected_params,
    base::Value* value,
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* return_value) {
  EXPECT_EQ(expected_id, session->id);
  EXPECT_TRUE(expected_params->Equals(&params));
  return_value->reset(value->DeepCopy());
  session->quit = true;
  return Status(kOk);
}

void OnSimpleCommand(base::RunLoop* run_loop,
                     const std::string& expected_session_id,
                     base::Value* expected_value,
                     const Status& status,
                     scoped_ptr<base::Value> value,
                     const std::string& session_id) {
  ASSERT_EQ(kOk, status.code());
  ASSERT_TRUE(expected_value->Equals(value.get()));
  ASSERT_EQ(expected_session_id, session_id);
  run_loop->Quit();
}

}  // namespace

TEST(CommandsTest, ExecuteSessionCommand) {
  SessionThreadMap map;
  linked_ptr<base::Thread> thread(new base::Thread("1"));
  ASSERT_TRUE(thread->Start());
  std::string id("id");
  thread->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&internal::CreateSessionOnSessionThreadForTesting, id));
  map[id] = thread;

  base::DictionaryValue params;
  params.SetInteger("param", 5);
  base::FundamentalValue expected_value(6);
  SessionCommand cmd = base::Bind(
      &ExecuteSimpleCommand, id, &params, &expected_value);

  base::MessageLoop loop;
  base::RunLoop run_loop;
  ExecuteSessionCommand(
      &map,
      "cmd",
      cmd,
      false,
      params,
      id,
      base::Bind(&OnSimpleCommand, &run_loop, id, &expected_value));
  run_loop.Run();
}

namespace {

Status ShouldNotBeCalled(
    Session* session,
    const base::DictionaryValue& params,
    scoped_ptr<base::Value>* value) {
  EXPECT_TRUE(false);
  return Status(kOk);
}

void OnNoSuchSession(const Status& status,
                     scoped_ptr<base::Value> value,
                     const std::string& session_id) {
  EXPECT_EQ(kNoSuchSession, status.code());
  EXPECT_FALSE(value.get());
}

void OnNoSuchSessionIsOk(const Status& status,
                         scoped_ptr<base::Value> value,
                         const std::string& session_id) {
  EXPECT_EQ(kOk, status.code());
  EXPECT_FALSE(value.get());
}

}  // namespace

TEST(CommandsTest, ExecuteSessionCommandOnNoSuchSession) {
  SessionThreadMap map;
  base::DictionaryValue params;
  ExecuteSessionCommand(&map,
                        "cmd",
                        base::Bind(&ShouldNotBeCalled),
                        false,
                        params,
                        "session",
                        base::Bind(&OnNoSuchSession));
}

TEST(CommandsTest, ExecuteSessionCommandOnNoSuchSessionWhenItExpectsOk) {
  SessionThreadMap map;
  base::DictionaryValue params;
  ExecuteSessionCommand(&map,
                        "cmd",
                        base::Bind(&ShouldNotBeCalled),
                        true,
                        params,
                        "session",
                        base::Bind(&OnNoSuchSessionIsOk));
}

namespace {

void OnNoSuchSessionAndQuit(base::RunLoop* run_loop,
                            const Status& status,
                            scoped_ptr<base::Value> value,
                            const std::string& session_id) {
  run_loop->Quit();
  EXPECT_EQ(kNoSuchSession, status.code());
  EXPECT_FALSE(value.get());
}

}  // namespace

TEST(CommandsTest, ExecuteSessionCommandOnJustDeletedSession) {
  SessionThreadMap map;
  linked_ptr<base::Thread> thread(new base::Thread("1"));
  ASSERT_TRUE(thread->Start());
  std::string id("id");
  map[id] = thread;

  base::MessageLoop loop;
  base::RunLoop run_loop;
  ExecuteSessionCommand(&map,
                        "cmd",
                        base::Bind(&ShouldNotBeCalled),
                        false,
                        base::DictionaryValue(),
                        "session",
                        base::Bind(&OnNoSuchSessionAndQuit, &run_loop));
  run_loop.Run();
}

namespace {

enum TestScenario {
  kElementExistsQueryOnce = 0,
  kElementExistsQueryTwice,
  kElementNotExistsQueryOnce,
  kElementExistsTimeout
};

class FindElementWebView : public StubWebView {
 public:
  FindElementWebView(bool only_one, TestScenario scenario)
      : StubWebView("1"), only_one_(only_one), scenario_(scenario),
        current_count_(0) {
    switch (scenario_) {
      case kElementExistsQueryOnce:
      case kElementExistsQueryTwice:
      case kElementExistsTimeout: {
        if (only_one_) {
          base::DictionaryValue element;
          element.SetString("ELEMENT", "1");
          result_.reset(element.DeepCopy());
        } else {
          base::DictionaryValue element1;
          element1.SetString("ELEMENT", "1");
          base::DictionaryValue element2;
          element2.SetString("ELEMENT", "2");
          base::ListValue list;
          list.Append(element1.DeepCopy());
          list.Append(element2.DeepCopy());
          result_.reset(list.DeepCopy());
        }
        break;
      }
      case kElementNotExistsQueryOnce: {
        if (only_one_)
          result_.reset(base::Value::CreateNullValue());
        else
          result_.reset(new base::ListValue());
        break;
      }
    }
  }
  virtual ~FindElementWebView() {}

  void Verify(const std::string& expected_frame,
              const base::ListValue* expected_args,
              const base::Value* actrual_result) {
    EXPECT_EQ(expected_frame, frame_);
    std::string function;
    if (only_one_)
      function = webdriver::atoms::asString(webdriver::atoms::FIND_ELEMENT);
    else
      function = webdriver::atoms::asString(webdriver::atoms::FIND_ELEMENTS);
    EXPECT_EQ(function, function_);
    ASSERT_TRUE(args_.get());
    EXPECT_TRUE(expected_args->Equals(args_.get()));
    ASSERT_TRUE(actrual_result);
    EXPECT_TRUE(result_->Equals(actrual_result));
  }

  // Overridden from WebView:
  virtual Status CallFunction(const std::string& frame,
                              const std::string& function,
                              const base::ListValue& args,
                              scoped_ptr<base::Value>* result) OVERRIDE {
    ++current_count_;
    if (scenario_ == kElementExistsTimeout ||
        (scenario_ == kElementExistsQueryTwice && current_count_ == 1)) {
        // Always return empty result when testing timeout.
        if (only_one_)
          result->reset(base::Value::CreateNullValue());
        else
          result->reset(new base::ListValue());
    } else {
      switch (scenario_) {
        case kElementExistsQueryOnce:
        case kElementNotExistsQueryOnce: {
          EXPECT_EQ(1, current_count_);
          break;
        }
        case kElementExistsQueryTwice: {
          EXPECT_EQ(2, current_count_);
          break;
        }
        default: {
          break;
        }
      }

      result->reset(result_->DeepCopy());
      frame_ = frame;
      function_ = function;
      args_.reset(args.DeepCopy());
    }
    return Status(kOk);
  }

 private:
  bool only_one_;
  TestScenario scenario_;
  int current_count_;
  std::string frame_;
  std::string function_;
  scoped_ptr<base::ListValue> args_;
  scoped_ptr<base::Value> result_;
};

}  // namespace

TEST(CommandsTest, SuccessfulFindElement) {
  FindElementWebView web_view(true, kElementExistsQueryTwice);
  Session session("id");
  session.implicit_wait = base::TimeDelta::FromSeconds(1);
  session.SwitchToSubFrame("frame_id1", std::string());
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  scoped_ptr<base::Value> result;
  ASSERT_EQ(kOk,
            ExecuteFindElement(1, &session, &web_view, params, &result).code());
  base::DictionaryValue param;
  param.SetString("id", "a");
  base::ListValue expected_args;
  expected_args.Append(param.DeepCopy());
  web_view.Verify("frame_id1", &expected_args, result.get());
}

TEST(CommandsTest, FailedFindElement) {
  FindElementWebView web_view(true, kElementNotExistsQueryOnce);
  Session session("id");
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  scoped_ptr<base::Value> result;
  ASSERT_EQ(kNoSuchElement,
            ExecuteFindElement(1, &session, &web_view, params, &result).code());
}

TEST(CommandsTest, SuccessfulFindElements) {
  FindElementWebView web_view(false, kElementExistsQueryTwice);
  Session session("id");
  session.implicit_wait = base::TimeDelta::FromSeconds(1);
  session.SwitchToSubFrame("frame_id2", std::string());
  base::DictionaryValue params;
  params.SetString("using", "name");
  params.SetString("value", "b");
  scoped_ptr<base::Value> result;
  ASSERT_EQ(
      kOk,
      ExecuteFindElements(1, &session, &web_view, params, &result).code());
  base::DictionaryValue param;
  param.SetString("name", "b");
  base::ListValue expected_args;
  expected_args.Append(param.DeepCopy());
  web_view.Verify("frame_id2", &expected_args, result.get());
}

TEST(CommandsTest, FailedFindElements) {
  Session session("id");
  FindElementWebView web_view(false, kElementNotExistsQueryOnce);
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  scoped_ptr<base::Value> result;
  ASSERT_EQ(
      kOk,
      ExecuteFindElements(1, &session, &web_view, params, &result).code());
  base::ListValue* list;
  ASSERT_TRUE(result->GetAsList(&list));
  ASSERT_EQ(0U, list->GetSize());
}

TEST(CommandsTest, SuccessfulFindChildElement) {
  FindElementWebView web_view(true, kElementExistsQueryTwice);
  Session session("id");
  session.implicit_wait = base::TimeDelta::FromSeconds(1);
  session.SwitchToSubFrame("frame_id3", std::string());
  base::DictionaryValue params;
  params.SetString("using", "tag name");
  params.SetString("value", "div");
  std::string element_id = "1";
  scoped_ptr<base::Value> result;
  ASSERT_EQ(
      kOk,
      ExecuteFindChildElement(
          1, &session, &web_view, element_id, params, &result).code());
  base::DictionaryValue locator_param;
  locator_param.SetString("tag name", "div");
  base::DictionaryValue root_element_param;
  root_element_param.SetString("ELEMENT", element_id);
  base::ListValue expected_args;
  expected_args.Append(locator_param.DeepCopy());
  expected_args.Append(root_element_param.DeepCopy());
  web_view.Verify("frame_id3", &expected_args, result.get());
}

TEST(CommandsTest, FailedFindChildElement) {
  Session session("id");
  FindElementWebView web_view(true, kElementNotExistsQueryOnce);
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  std::string element_id = "1";
  scoped_ptr<base::Value> result;
  ASSERT_EQ(
      kNoSuchElement,
      ExecuteFindChildElement(
          1, &session, &web_view, element_id, params, &result).code());
}

TEST(CommandsTest, SuccessfulFindChildElements) {
  FindElementWebView web_view(false, kElementExistsQueryTwice);
  Session session("id");
  session.implicit_wait = base::TimeDelta::FromSeconds(1);
  session.SwitchToSubFrame("frame_id4", std::string());
  base::DictionaryValue params;
  params.SetString("using", "class name");
  params.SetString("value", "c");
  std::string element_id = "1";
  scoped_ptr<base::Value> result;
  ASSERT_EQ(
      kOk,
      ExecuteFindChildElements(
          1, &session, &web_view, element_id, params, &result).code());
  base::DictionaryValue locator_param;
  locator_param.SetString("class name", "c");
  base::DictionaryValue root_element_param;
  root_element_param.SetString("ELEMENT", element_id);
  base::ListValue expected_args;
  expected_args.Append(locator_param.DeepCopy());
  expected_args.Append(root_element_param.DeepCopy());
  web_view.Verify("frame_id4", &expected_args, result.get());
}

TEST(CommandsTest, FailedFindChildElements) {
  Session session("id");
  FindElementWebView web_view(false, kElementNotExistsQueryOnce);
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  std::string element_id = "1";
  scoped_ptr<base::Value> result;
  ASSERT_EQ(
      kOk,
      ExecuteFindChildElements(
          1, &session, &web_view, element_id, params, &result).code());
  base::ListValue* list;
  ASSERT_TRUE(result->GetAsList(&list));
  ASSERT_EQ(0U, list->GetSize());
}

TEST(CommandsTest, TimeoutInFindElement) {
  Session session("id");
  FindElementWebView web_view(true, kElementExistsTimeout);
  session.implicit_wait = base::TimeDelta::FromMilliseconds(2);
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  params.SetString("id", "1");
  scoped_ptr<base::Value> result;
  ASSERT_EQ(kNoSuchElement,
            ExecuteFindElement(1, &session, &web_view, params, &result).code());
}

namespace {

class ErrorCallFunctionWebView : public StubWebView {
 public:
  explicit ErrorCallFunctionWebView(StatusCode code)
      : StubWebView("1"), code_(code) {}
  virtual ~ErrorCallFunctionWebView() {}

  // Overridden from WebView:
  virtual Status CallFunction(const std::string& frame,
                              const std::string& function,
                              const base::ListValue& args,
                              scoped_ptr<base::Value>* result) OVERRIDE {
    return Status(code_);
  }

 private:
  StatusCode code_;
};

}  // namespace

TEST(CommandsTest, ErrorFindElement) {
  Session session("id");
  ErrorCallFunctionWebView web_view(kUnknownError);
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  scoped_ptr<base::Value> value;
  ASSERT_EQ(kUnknownError,
            ExecuteFindElement(1, &session, &web_view, params, &value).code());
  ASSERT_EQ(kUnknownError,
            ExecuteFindElements(1, &session, &web_view, params, &value).code());
}

TEST(CommandsTest, ErrorFindChildElement) {
  Session session("id");
  ErrorCallFunctionWebView web_view(kStaleElementReference);
  base::DictionaryValue params;
  params.SetString("using", "id");
  params.SetString("value", "a");
  std::string element_id = "1";
  scoped_ptr<base::Value> result;
  ASSERT_EQ(
      kStaleElementReference,
      ExecuteFindChildElement(
          1, &session, &web_view, element_id, params, &result).code());
  ASSERT_EQ(
      kStaleElementReference,
      ExecuteFindChildElements(
          1, &session, &web_view, element_id, params, &result).code());
}
