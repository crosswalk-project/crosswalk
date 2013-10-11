// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

#include "testing/gtest/include/gtest/gtest.h"

using xwalk::extensions::XWalkExtensionFunctionHandler;
using xwalk::extensions::XWalkExtensionFunctionInfo;

namespace {

const char kTestString[] = "crosswalk1234567890";

void DispatchResult(std::string* str, scoped_ptr<base::ListValue> result) {
  result->GetString(0, str);
}

void StoreFunctionInfo(XWalkExtensionFunctionInfo** info_ptr,
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  *info_ptr = info.release();
}

void EchoData(int* counter, scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string str;
  info->arguments()->GetString(0, &str);
  EXPECT_EQ(str, kTestString);

  scoped_ptr<base::ListValue> result(new base::ListValue());
  result->AppendString(str);

  info->PostResult(result.Pass());

  (*counter)++;
}

void ResetCounter(int* counter, scoped_ptr<XWalkExtensionFunctionInfo> info) {
  *counter = 0;
}

}  // namespace

TEST(XWalkExtensionFunctionHandlerTest, PostResult) {
  std::string str;

  XWalkExtensionFunctionInfo info(
      "test",
      make_scoped_ptr(new base::ListValue()).Pass(),
      base::Bind(&DispatchResult, &str));

  scoped_ptr<base::ListValue> data(new base::ListValue());
  data->AppendString(kTestString);

  info.PostResult(data.Pass());
  EXPECT_EQ(str, kTestString);
}

TEST(XWalkExtensionFunctionHandlerTest, RegisterAndHandleFunction) {
  XWalkExtensionFunctionHandler handler(NULL);

  int counter = 0;
  handler.Register("echoData", base::Bind(&EchoData, &counter));
  handler.Register("reset", base::Bind(&ResetCounter, &counter));

  for (unsigned i = 0; i < 1000; ++i) {
    std::string str1;
    scoped_ptr<base::ListValue> data1(new base::ListValue());
    data1->AppendString(kTestString);

    scoped_ptr<XWalkExtensionFunctionInfo> info1(new XWalkExtensionFunctionInfo(
        "echoData",
        data1.Pass(),
        base::Bind(&DispatchResult, &str1)));

    handler.HandleFunction(info1.Pass());
    EXPECT_EQ(counter, i + 1);
    EXPECT_EQ(str1, kTestString);
  }

  std::string str2;
  scoped_ptr<base::ListValue> data2(new base::ListValue());
  data2->AppendString(kTestString);

  scoped_ptr<XWalkExtensionFunctionInfo> info2(new XWalkExtensionFunctionInfo(
      "reset",
      data2.Pass(),
      base::Bind(&DispatchResult, &str2)));

  handler.HandleFunction(info2.Pass());
  EXPECT_EQ(counter, 0);

  // Dispatching to a non registered handler should not crash.
  std::string str3;
  scoped_ptr<base::ListValue> data3(new base::ListValue());
  data3->AppendString(kTestString);

  scoped_ptr<XWalkExtensionFunctionInfo> info3(new XWalkExtensionFunctionInfo(
      "foobar",
      data3.Pass(),
      base::Bind(&DispatchResult, &str3)));

  handler.HandleFunction(info3.Pass());
}

TEST(XWalkExtensionFunctionHandlerTest, PostingResultAfterDeletingTheHandler) {
  scoped_ptr<XWalkExtensionFunctionHandler> handler(
      new XWalkExtensionFunctionHandler(NULL));

  XWalkExtensionFunctionInfo* info;
  handler->Register("storeFunctionInfo", base::Bind(&StoreFunctionInfo, &info));

  scoped_ptr<base::ListValue> msg(new base::ListValue);
  msg->AppendString("storeFunctionInfo");  // Function name.
  msg->AppendString("id");  // Callback ID.

  handler->HandleMessage(msg.PassAs<base::Value>());
  handler.reset();

  // Posting a result after deleting the handler should not
  // crash because internally, the reference to the handler
  // is weak.
  info->PostResult(make_scoped_ptr(new base::ListValue));
  delete info;
}
