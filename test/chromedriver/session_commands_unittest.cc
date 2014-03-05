// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/stub_chrome.h"
#include "xwalk/test/chromedriver/commands.h"
#include "xwalk/test/chromedriver/session.h"
#include "xwalk/test/chromedriver/session_commands.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(SessionCommandTest, FileUpload) {
  Session session("id");
  base::DictionaryValue params;
  scoped_ptr<base::Value> value;
  // Zip file entry that contains a single file with contents 'COW\n', base64
  // encoded following RFC 1521.
  const char* kBase64ZipEntry =
      "UEsDBBQAAAAAAMROi0K/wAzGBAAAAAQAAAADAAAAbW9vQ09XClBLAQIUAxQAAAAAAMROi0K/"
      "wAzG\nBAAAAAQAAAADAAAAAAAAAAAAAACggQAAAABtb29QSwUGAAAAAAEAAQAxAAAAJQAAAA"
      "AA\n";
  params.SetString("file", kBase64ZipEntry);
  Status status = ExecuteUploadFile(&session, params, &value);
  ASSERT_EQ(kOk, status.code()) << status.message();
  base::FilePath::StringType path;
  ASSERT_TRUE(value->GetAsString(&path));
  ASSERT_TRUE(base::PathExists(base::FilePath(path)));
  std::string data;
  ASSERT_TRUE(base::ReadFileToString(base::FilePath(path), &data));
  ASSERT_STREQ("COW\n", data.c_str());
}

namespace {

class DetachChrome : public StubChrome {
 public:
  DetachChrome() : quit_called_(false) {}
  virtual ~DetachChrome() {}

  // Overridden from Chrome:
  virtual Status Quit() OVERRIDE {
    quit_called_ = true;
    return Status(kOk);
  }

  bool quit_called_;
};

}  // namespace

TEST(SessionCommandsTest, Quit) {
  DetachChrome* chrome = new DetachChrome();
  Session session("id", scoped_ptr<Chrome>(chrome));

  base::DictionaryValue params;
  scoped_ptr<base::Value> value;

  ASSERT_EQ(kOk, ExecuteQuit(false, &session, params, &value).code());
  ASSERT_TRUE(chrome->quit_called_);

  chrome->quit_called_ = false;
  ASSERT_EQ(kOk, ExecuteQuit(true, &session, params, &value).code());
  ASSERT_TRUE(chrome->quit_called_);
}

TEST(SessionCommandsTest, QuitWithDetach) {
  DetachChrome* chrome = new DetachChrome();
  Session session("id", scoped_ptr<Chrome>(chrome));
  session.detach = true;

  base::DictionaryValue params;
  scoped_ptr<base::Value> value;

  ASSERT_EQ(kOk, ExecuteQuit(true, &session, params, &value).code());
  ASSERT_FALSE(chrome->quit_called_);

  ASSERT_EQ(kOk, ExecuteQuit(false, &session, params, &value).code());
  ASSERT_TRUE(chrome->quit_called_);
}

namespace {

class FailsToQuitChrome : public StubChrome {
 public:
  FailsToQuitChrome() {}
  virtual ~FailsToQuitChrome() {}

  // Overridden from Chrome:
  virtual Status Quit() OVERRIDE {
    return Status(kUnknownError);
  }
};

}  // namespace

TEST(SessionCommandsTest, QuitFails) {
  Session session("id", scoped_ptr<Chrome>(new FailsToQuitChrome()));
  base::DictionaryValue params;
  scoped_ptr<base::Value> value;
  ASSERT_EQ(kUnknownError, ExecuteQuit(false, &session, params, &value).code());
}
