// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TEST_APPLICATION_TESTAPI_H_
#define XWALK_APPLICATION_TEST_APPLICATION_TESTAPI_H_

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "xwalk/extensions/common/xwalk_extension.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionFunctionHandler;
using xwalk::extensions::XWalkExtensionFunctionInfo;
using xwalk::extensions::XWalkExtensionInstance;

namespace content {
class MessageLoopRunner;
}

class ApiTestExtensionInstance : public XWalkExtensionInstance {
 public:
  // Observer will be created in UI thread.
  class Observer {
   public:
    enum Result {
      NOT_SET = 0,
      PASS,
      FAILURE,
    };
    virtual void NotifyTestComplete(Result result) = 0;
   protected:
    virtual ~Observer() {}
  };

  explicit ApiTestExtensionInstance(Observer* observer = NULL);

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  void OnNotifyPass(scoped_ptr<XWalkExtensionFunctionInfo> info);
  void OnNotifyFail(scoped_ptr<XWalkExtensionFunctionInfo> info);

  Observer* observer_;
  XWalkExtensionFunctionHandler handler_;
};

class ApiTestExtension : public XWalkExtension {
 public:
  ApiTestExtension();

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE;

  void SetObserver(ApiTestExtensionInstance::Observer* observer);

 private:
  ApiTestExtensionInstance::Observer* observer_;
};

class ApiTestRunner : public ApiTestExtensionInstance::Observer {
 public:
  typedef ApiTestExtensionInstance::Observer::Result Result;

  ApiTestRunner();
  ~ApiTestRunner();

  // Block wait until the test API is called. If the test API is already called,
  // this will return immediately. Returns true if the waiting happened, returns
  // false if the waiting does not happen.
  bool WaitForTestComplete();

  // Implement ApiTestExtensionInstance::Observer.
  virtual void NotifyTestComplete(Result result) OVERRIDE;

  Result GetTestsResult() const;

  // Reset current test result, then it can wait again.
  void ResetResult();

 private:
  Result result_;
  scoped_refptr<content::MessageLoopRunner> message_loop_runner_;
};

#endif  // XWALK_APPLICATION_TEST_APPLICATION_TESTAPI_H_
