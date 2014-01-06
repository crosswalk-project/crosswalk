// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/test/application_testapi.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/test/test_utils.h"
#include "grit/xwalk_application_resources.h"
#include "ui/base/resource/resource_bundle.h"

using content::BrowserThread;

namespace {
const char kTestApiName[] = "xwalk.app.test";

const char kTestNotifyPass[] = "notifyPass";
const char kTestNotifyFail[] = "notifyFail";
const char kTestNotifyTimeout[] = "notifyTimeout";

}  // namespace

ApiTestExtension::ApiTestExtension() {
  set_name(kTestApiName);
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_APPLICATION_TEST_API).as_string());
}

xwalk::extensions::XWalkExtensionInstance* ApiTestExtension::CreateInstance() {
  return new ApiTestExtensionInstance(observer_);
}

void ApiTestExtension::SetObserver(
    ApiTestExtensionInstance::Observer* observer) {
  observer_ = observer;
}

ApiTestExtensionInstance::ApiTestExtensionInstance(
    ApiTestExtensionInstance::Observer* observer)
  : observer_(observer),
    handler_(this) {
  handler_.Register(
      kTestNotifyPass,
      base::Bind(&ApiTestExtensionInstance::OnNotifyPass,
                 base::Unretained(this)));
  handler_.Register(
      kTestNotifyFail,
      base::Bind(&ApiTestExtensionInstance::OnNotifyFail,
                 base::Unretained(this)));
  handler_.Register(
      kTestNotifyTimeout,
      base::Bind(&ApiTestExtensionInstance::OnNotifyTimeout,
                 base::Unretained(this)));
}

void ApiTestExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void ApiTestExtensionInstance::OnNotifyPass(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  CHECK(observer_);
  observer_->OnTestNotificationReceived(info.Pass(), kTestNotifyPass);
}

void ApiTestExtensionInstance::OnNotifyFail(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  CHECK(observer_);
  observer_->OnTestNotificationReceived(info.Pass(), kTestNotifyFail);
}

void ApiTestExtensionInstance::OnNotifyTimeout(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  CHECK(observer_);
  observer_->OnTestNotificationReceived(info.Pass(), kTestNotifyTimeout);
}

ApiTestRunner::ApiTestRunner()
  : result_(NOT_SET) {
}

ApiTestRunner::~ApiTestRunner() {
}

bool ApiTestRunner::WaitForTestNotification() {
  if (result_ != NOT_SET || message_loop_runner_)
    return false;

  message_loop_runner_ = new content::MessageLoopRunner;
  message_loop_runner_->Run();
  message_loop_runner_ = NULL;
  return true;
}

void ApiTestRunner::OnTestNotificationReceived(
    scoped_ptr<XWalkExtensionFunctionInfo> info,
    const std::string& result_str) {
  notify_func_info_.reset(info.release());
  Result result = NOT_SET;
  if (result_str == kTestNotifyPass)
    result = PASS;
  else if (result_str == kTestNotifyFail)
    result = FAILURE;
  else if (result_str == kTestNotifyTimeout)
    result = TIMEOUT;
  else
    NOTREACHED();

  CHECK(result != NOT_SET);
  result_ = result;
  // It may be notified before wait really happens.
  if (!message_loop_runner_)
    return;

  message_loop_runner_->Quit();
}

void ApiTestRunner::PostResultToNotificationCallback() {
  ResetResult();
  notify_func_info_->PostResult(
      scoped_ptr<base::ListValue>(new base::ListValue));
}

ApiTestRunner::Result ApiTestRunner::GetTestsResult() const {
  return result_;
}

void ApiTestRunner::ResetResult() {
  CHECK(!message_loop_runner_);
  result_ = NOT_SET;
}
