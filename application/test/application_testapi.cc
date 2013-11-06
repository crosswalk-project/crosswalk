// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/test/application_testapi.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/test/test_utils.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/grit/xwalk_application_resources.h"

using content::BrowserThread;

namespace {
const char kTestApiName[] = "xwalk.app.test";

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
      "notifyPass",
      base::Bind(&ApiTestExtensionInstance::OnNotifyPass,
                 base::Unretained(this)));
  handler_.Register(
      "notifyFail",
      base::Bind(&ApiTestExtensionInstance::OnNotifyFail,
                 base::Unretained(this)));
}

void ApiTestExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void ApiTestExtensionInstance::OnNotifyPass(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  CHECK(observer_);
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&Observer::NotifyTestComplete,
                 base::Unretained(observer_),
                 Observer::PASS));

  scoped_ptr<base::ListValue> results(new base::ListValue());
  info->PostResult(results.Pass());
}

void ApiTestExtensionInstance::OnNotifyFail(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  CHECK(observer_);
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&Observer::NotifyTestComplete,
                 base::Unretained(observer_),
                 Observer::FAILURE));

  scoped_ptr<base::ListValue> results(new base::ListValue());
  info->PostResult(results.Pass());
}

ApiTestRunner::ApiTestRunner()
  : result_(NOT_SET) {
}

ApiTestRunner::~ApiTestRunner() {
}

bool ApiTestRunner::WaitForTestComplete() {
  if (result_ != NOT_SET || message_loop_runner_)
    return false;

  message_loop_runner_ = new content::MessageLoopRunner;
  message_loop_runner_->Run();
  message_loop_runner_ = NULL;
  return true;
}

void ApiTestRunner::NotifyTestComplete(ApiTestRunner::Result result) {
  CHECK(result != NOT_SET);
  result_ = result;
  // It may be notified before wait really happens.
  if (!message_loop_runner_)
    return;

  message_loop_runner_->Quit();
}

ApiTestRunner::Result ApiTestRunner::GetTestsResult() const {
  return result_;
}

void ApiTestRunner::ResetResult() {
  CHECK(!message_loop_runner_);
  result_ = NOT_SET;
}
