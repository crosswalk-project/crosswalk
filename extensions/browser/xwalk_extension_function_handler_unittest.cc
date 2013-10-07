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

void EchoData(int* counter, const XWalkExtensionFunctionInfo& info) {
  std::string str;
  info.arguments->GetString(0, &str);
  EXPECT_EQ(str, kTestString);

  scoped_ptr<base::ListValue> result(new base::ListValue());
  result->AppendString(str);

  info.PostResult(result.Pass());

  (*counter)++;
}

void ResetCounter(int* counter, const XWalkExtensionFunctionInfo& info) {
  *counter = 0;
}

}  // namespace

TEST(XWalkExtensionFunctionHandlerTest, PostResult) {
  XWalkExtensionFunctionInfo info;

  std::string str;
  info.post_result_cb = base::Bind(&DispatchResult, &str);

  scoped_ptr<base::ListValue> data(new base::ListValue());
  data->AppendString(kTestString);

  info.PostResult(data.Pass());
  EXPECT_EQ(str, kTestString);
}

TEST(XWalkExtensionFunctionHandlerTest, RegisterAndHandleFunction) {
  XWalkExtensionFunctionHandler handler;

  int counter = 0;
  handler.Register("echoData", base::Bind(&EchoData, &counter));
  handler.Register("reset", base::Bind(&ResetCounter, &counter));

  XWalkExtensionFunctionInfo info;
  info.name = "echoData";

  scoped_ptr<base::ListValue> data(new base::ListValue());
  data->AppendString(kTestString);
  info.arguments = data.get();

  std::string str;
  info.post_result_cb = base::Bind(&DispatchResult, &str);

  for (unsigned i = 0; i < 1000; ++i) {
    handler.HandleFunction(info);
    EXPECT_EQ(counter, i + 1);
    EXPECT_EQ(str, kTestString);
  }

  info.name = "reset";
  handler.HandleFunction(info);
  EXPECT_EQ(counter, 0);

  // Dispatching to a non registered handler should not crash.
  info.name = "foobar";
  handler.HandleFunction(info);
}
