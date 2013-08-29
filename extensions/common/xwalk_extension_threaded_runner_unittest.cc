// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/xwalk_extension_threaded_runner.h"

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/run_loop.h"
#include "base/synchronization/waitable_event.h"
#include "ipc/ipc_message.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::MessageLoop;
using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionInstance;
using xwalk::extensions::XWalkExtensionRunner;
using xwalk::extensions::XWalkExtensionThreadedRunner;

namespace {

MessageLoop* g_main_message_loop = NULL;
MessageLoop* g_extension_message_loop = NULL;
base::WaitableEvent g_done(false, false);

class TestExtensionInstance : public XWalkExtensionInstance {
 public:
  TestExtensionInstance(
      const XWalkExtension::PostMessageCallback post_message)
      : XWalkExtensionInstance(post_message),
        extension_message_loop_(MessageLoop::current()) {
    EXPECT_NE(g_main_message_loop, extension_message_loop_);
    g_done.Signal();
  }
  virtual ~TestExtensionInstance() {
    EXPECT_EQ(extension_message_loop_, MessageLoop::current());
    g_done.Signal();
  }

 private:
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {
    EXPECT_EQ(extension_message_loop_, MessageLoop::current());
    std::string msg_str;
    msg->GetAsString(&msg_str);
    if (msg_str == "PING") {
      PostMessage(scoped_ptr<base::Value>(
          base::Value::CreateStringValue("PONG")));
    } else {
      g_done.Signal();
    }
  }
  virtual scoped_ptr<base::Value> HandleSyncMessage(
      scoped_ptr<base::Value> msg) OVERRIDE {
    EXPECT_EQ(extension_message_loop_, MessageLoop::current());
    g_done.Signal();
    return scoped_ptr<base::Value>();
  }

  MessageLoop* extension_message_loop_;
};

class TestExtension : public XWalkExtension {
 public:
  TestExtension() {}
  virtual const char* GetJavaScriptAPI() OVERRIDE { return ""; }
  virtual XWalkExtensionInstance* CreateInstance(
      const PostMessageCallback& post_message) OVERRIDE {
    return new TestExtensionInstance(post_message);
  }
};

class TestRunnerClient : public XWalkExtensionRunner::Client {
 public:
  TestRunnerClient(const base::Closure& handle_message = base::Closure())
      : handle_message_(handle_message) {}

  virtual void HandleMessageFromContext(
      const XWalkExtensionRunner* runner,
      scoped_ptr<base::Value> msg) OVERRIDE {
    EXPECT_EQ(g_main_message_loop, MessageLoop::current());

    // Posting instead of calling directly because if the expectation above
    // fails, we will be in the wrong thread.
    if (!handle_message_.is_null()) {
      g_main_message_loop->message_loop_proxy()->PostTask(
          FROM_HERE, handle_message_);
    }
  }

  virtual void HandleReplyMessageFromContext(scoped_ptr<IPC::Message> ipc_reply,
      scoped_ptr<base::Value> msg) OVERRIDE {
    EXPECT_EQ(g_main_message_loop, MessageLoop::current());

    // Posting instead of calling directly because if the expectation above
    // fails, we will be in the wrong thread.
    if (!handle_message_.is_null()) {
      g_main_message_loop->message_loop_proxy()->PostTask(
          FROM_HERE, handle_message_);
    }
  }

 private:
  base::Closure handle_message_;
};

}  // namespace

TEST(XWalkExtensionThreadedRunnerTest,
     ContextFunctionsCalledInExtensionThread) {
  MessageLoop loop(MessageLoop::TYPE_DEFAULT);
  g_main_message_loop = &loop;

  TestExtension extension;
  TestRunnerClient client;

  // We test over the XWalkExtensionRunner interface, which is the
  // interface used elsewhere in the code.
  XWalkExtensionRunner* runner =
      new XWalkExtensionThreadedRunner(&extension, &client,
                                       loop.message_loop_proxy());
  g_done.Wait();

  runner->PostMessageToContext(scoped_ptr<base::Value>(
      base::Value::CreateStringValue("HELLO")));
  g_done.Wait();


  runner->SendSyncMessageToContext(make_scoped_ptr(new IPC::Message),
      scoped_ptr<base::Value>(base::Value::CreateStringValue("HELLO")));
  g_done.Wait();

  delete runner;
  g_done.Wait();

  g_main_message_loop = NULL;
}

TEST(XWalkExtensionThreadedRunnerTest,
     ClientFunctionNotCalledInExtensionThread) {
  MessageLoop loop(MessageLoop::TYPE_DEFAULT);
  g_main_message_loop = &loop;
  base::RunLoop run_loop;

  TestExtension extension;
  TestRunnerClient client(run_loop.QuitClosure());

  XWalkExtensionRunner* runner =
      new XWalkExtensionThreadedRunner(&extension, &client,
                                       loop.message_loop_proxy());
  g_done.Wait();

  EXPECT_FALSE(g_done.IsSignaled());

  runner->PostMessageToContext(scoped_ptr<base::Value>(
      base::Value::CreateStringValue("PING")));

  run_loop.Run();

  delete runner;
  g_done.Wait();

  g_main_message_loop = NULL;
}

TEST(XWalkExtensionThreadedRunnerTest,
     DoNotCrashWhenHavePendingMessageToClient) {
  MessageLoop loop(MessageLoop::TYPE_DEFAULT);
  g_main_message_loop = &loop;

  TestExtension extension;
  TestRunnerClient client;

  XWalkExtensionRunner* runner =
      new XWalkExtensionThreadedRunner(&extension, &client,
                                       loop.message_loop_proxy());
  g_done.Wait();

  runner->PostMessageToContext(scoped_ptr<base::Value>(
      base::Value::CreateStringValue("PING")));
  delete runner;
  g_done.Wait();

  base::RunLoop run_loop;
  run_loop.RunUntilIdle();

  g_main_message_loop = NULL;
}
