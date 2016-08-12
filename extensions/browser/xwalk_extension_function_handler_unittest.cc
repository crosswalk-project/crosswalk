// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"

#include "base/memory/ptr_util.h"
#include "testing/gtest/include/gtest/gtest.h"

using xwalk::extensions::XWalkExtensionFunctionHandler;
using xwalk::extensions::XWalkExtensionFunctionInfo;

namespace {

const char kTestString[] = "crosswalk1234567890";

void DispatchResult(std::string* str, std::unique_ptr<base::ListValue> result) {
  result->GetString(0, str);
}

void StoreFunctionInfo(XWalkExtensionFunctionInfo** info_ptr,
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  *info_ptr = info.release();
}

void EchoData(int* counter, std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::string str;
  info->arguments()->GetString(0, &str);
  EXPECT_EQ(str, kTestString);

  std::unique_ptr<base::ListValue> result(new base::ListValue());
  result->AppendString(str);

  info->PostResult(std::move(result));

  (*counter)++;
}

void ResetCounter(int* counter, std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  *counter = 0;
}

}  // namespace

TEST(XWalkExtensionFunctionHandlerTest, PostResult) {
  std::string str;

  XWalkExtensionFunctionInfo info(
      "test",
      base::WrapUnique(new base::ListValue()),
      base::Bind(&DispatchResult, &str));

  std::unique_ptr<base::ListValue> data(new base::ListValue());
  data->AppendString(kTestString);

  info.PostResult(std::move(data));
  EXPECT_EQ(str, kTestString);
}

TEST(XWalkExtensionFunctionHandlerTest, RegisterAndHandleFunction) {
  XWalkExtensionFunctionHandler handler(NULL);

  int counter = 0;
  handler.Register("echoData", base::Bind(&EchoData, &counter));
  handler.Register("reset", base::Bind(&ResetCounter, &counter));

  for (int i = 0; i < 1000; ++i) {
    std::string str1;
    std::unique_ptr<base::ListValue> data1(new base::ListValue());
    data1->AppendString(kTestString);

    std::unique_ptr<XWalkExtensionFunctionInfo> info1(new XWalkExtensionFunctionInfo(
        "echoData",
        std::move(data1),
        base::Bind(&DispatchResult, &str1)));

    handler.HandleFunction(std::move(info1));
    EXPECT_EQ(counter, i + 1);
    EXPECT_EQ(str1, kTestString);
  }

  std::string str2;
  std::unique_ptr<base::ListValue> data2(new base::ListValue());
  data2->AppendString(kTestString);

  std::unique_ptr<XWalkExtensionFunctionInfo> info2(new XWalkExtensionFunctionInfo(
      "reset",
      std::move(data2),
      base::Bind(&DispatchResult, &str2)));

  handler.HandleFunction(std::move(info2));
  EXPECT_EQ(counter, 0);

  // Dispatching to a non registered handler should not crash.
  std::string str3;
  std::unique_ptr<base::ListValue> data3(new base::ListValue());
  data3->AppendString(kTestString);

  std::unique_ptr<XWalkExtensionFunctionInfo> info3(new XWalkExtensionFunctionInfo(
      "foobar",
      std::move(data3),
      base::Bind(&DispatchResult, &str3)));

  handler.HandleFunction(std::move(info3));
}

TEST(XWalkExtensionFunctionHandlerTest, PostingResultAfterDeletingTheHandler) {
  std::unique_ptr<XWalkExtensionFunctionHandler> handler(
      new XWalkExtensionFunctionHandler(NULL));

  XWalkExtensionFunctionInfo* info;
  handler->Register("storeFunctionInfo", base::Bind(&StoreFunctionInfo, &info));

  std::unique_ptr<base::ListValue> msg(new base::ListValue);
  msg->AppendString("storeFunctionInfo");  // Function name.
  msg->AppendString("id");  // Callback ID.

  handler->HandleMessage(std::move(msg));
  handler.reset();

  // Posting a result after deleting the handler should not
  // crash because internally, the reference to the handler
  // is weak.
  info->PostResult(base::WrapUnique(new base::ListValue));
  delete info;
}
