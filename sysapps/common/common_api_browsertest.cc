// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/common_api_browsertest.h"

#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/filename_util.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/sysapps/common/binding_object.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using namespace xwalk::extensions;  // NOLINT
using xwalk::sysapps::BindingObject;

SysAppsTestExtension::SysAppsTestExtension() {
  set_name("sysapps_common_test");
  set_javascript_api(
    "var v8tools = requireNative('v8tools');"
    ""
    "var internal = requireNative('internal');"
    "internal.setupInternalExtension(extension);"
    ""
    "var common = requireNative('sysapps_common');"
    "common.setupSysAppsCommon(internal, v8tools);"
    ""
    "var TestObject = function() {"
    "  common.BindingObject.call(this, common.getUniqueId());"
    "  common.EventTarget.call(this);"
    "  internal.postMessage('TestObjectConstructor', [this._id]);"
    "  this._addMethod('isTestEventActive', true);"
    "  this._addMethod('fireTestEvent', true);"
    "  this._addMethodWithPromise('makeFulfilledPromise');"
    "  this._addMethodWithPromise('makeRejectedPromise');"
    "  this._addEvent('test');"
    "  this._registerLifecycleTracker();"
    "};"
    ""
    "TestObject.prototype = new common.EventTargetPrototype();"
    ""
    "exports.v8tools = v8tools;"
    "exports.TestObject = TestObject;"
    "exports.hasObject = function(object_id, callback) {"
    "  internal.postMessage('hasObject', [object_id], callback);"
    "};");
}

XWalkExtensionInstance* SysAppsTestExtension::CreateInstance() {
  return new SysAppsTestExtensionInstance();
}

SysAppsTestExtensionInstance::SysAppsTestExtensionInstance()
  : handler_(this),
    store_(&handler_) {
  handler_.Register("TestObjectConstructor",
      base::Bind(&SysAppsTestExtensionInstance::OnSysAppsTestObjectContructor,
                 base::Unretained(this)));
  handler_.Register("hasObject",
      base::Bind(&SysAppsTestExtensionInstance::OnHasObject,
                 base::Unretained(this)));
}

void SysAppsTestExtensionInstance::HandleMessage(std::unique_ptr<base::Value> msg) {
  handler_.HandleMessage(std::move(msg));
}

void SysAppsTestExtensionInstance::OnSysAppsTestObjectContructor(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::string object_id;
  ASSERT_TRUE(info->arguments()->GetString(0, &object_id));

  std::unique_ptr<BindingObject> obj(new SysAppsTestObject);
  store_.AddBindingObject(object_id, std::move(obj));
}

void SysAppsTestExtensionInstance::OnHasObject(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::string object_id;
  ASSERT_TRUE(info->arguments()->GetString(0, &object_id));

  std::unique_ptr<base::ListValue> result(new base::ListValue());
  result->AppendBoolean(store_.HasObjectForTesting(object_id));

  info->PostResult(std::move(result));
}

SysAppsTestObject::SysAppsTestObject() : is_test_event_active_(false) {
  handler_.Register("isTestEventActive",
      base::Bind(&SysAppsTestObject::OnIsTestEventActive,
                 base::Unretained(this)));
  handler_.Register("fireTestEvent",
      base::Bind(&SysAppsTestObject::OnFireTestEvent,
                 base::Unretained(this)));
  handler_.Register("makeFulfilledPromise",
      base::Bind(&SysAppsTestObject::OnMakeFulfilledPromise,
                 base::Unretained(this)));
  handler_.Register("makeRejectedPromise",
      base::Bind(&SysAppsTestObject::OnMakeRejectedPromise,
                 base::Unretained(this)));
}

void SysAppsTestObject::StartEvent(const std::string& type) {
  if (type == "test")
    is_test_event_active_ = true;
}

void SysAppsTestObject::StopEvent(const std::string& type) {
  if (type == "test")
    is_test_event_active_ = false;
}

void SysAppsTestObject::OnIsTestEventActive(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<base::ListValue> result(new base::ListValue());
  result->AppendBoolean(is_test_event_active_);

  info->PostResult(std::move(result));
}

void SysAppsTestObject::OnFireTestEvent(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<base::ListValue> data(new base::ListValue());
  data->AppendString("Lorem ipsum");
  DispatchEvent("test", std::move(data));

  std::unique_ptr<base::ListValue> result(new base::ListValue());
  info->PostResult(std::move(result));
}

void SysAppsTestObject::OnMakeFulfilledPromise(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<base::ListValue> result(new base::ListValue());
  result->AppendString("Lorem ipsum");  // Data.
  result->AppendString("");  // Error, empty == no error.

  info->PostResult(std::move(result));
}

void SysAppsTestObject::OnMakeRejectedPromise(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<base::ListValue> result(new base::ListValue());
  result->AppendString("");  // Data.
  result->AppendString("Lorem ipsum");  // Error, !empty == error.

  info->PostResult(std::move(result));
}

class SysAppsCommonTest : public InProcessBrowserTest {
 public:
  void SetUp() override {
    XWalkExtensionService::SetCreateUIThreadExtensionsCallbackForTesting(
        base::Bind(&SysAppsCommonTest::CreateExtensions,
                   base::Unretained(this)));
    InProcessBrowserTest::SetUp();
  }

  void CreateExtensions(XWalkExtensionVector* extensions) {
    extensions->push_back(new SysAppsTestExtension);
  }
};

IN_PROC_BROWSER_TEST_F(SysAppsCommonTest, SysAppsCommon) {
  const base::string16 passString = base::ASCIIToUTF16("Pass");
  const base::string16 failString = base::ASCIIToUTF16("Fail");

  xwalk::Runtime* runtime = CreateRuntime();
  content::TitleWatcher title_watcher(runtime->web_contents(), passString);
  title_watcher.AlsoWaitForTitle(failString);

  base::FilePath test_file;
  PathService::Get(base::DIR_SOURCE_ROOT, &test_file);
  test_file = test_file
      .Append(FILE_PATH_LITERAL("xwalk"))
      .Append(FILE_PATH_LITERAL("sysapps"))
      .Append(FILE_PATH_LITERAL("common"))
      .Append(FILE_PATH_LITERAL("common_api_browsertest.html"));

  xwalk_test_utils::NavigateToURL(runtime, net::FilePathToFileURL(test_file));
  EXPECT_EQ(passString, title_watcher.WaitAndGetTitle());
}
