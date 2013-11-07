// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/common_api_browsertest.h"

#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/sysapps/common/binding_object.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using xwalk::extensions::XWalkExtension;
using xwalk::extensions::XWalkExtensionServer;
using xwalk::extensions::XWalkExtensionService;
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

void SysAppsTestExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void SysAppsTestExtensionInstance::OnSysAppsTestObjectContructor(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string object_id;
  ASSERT_TRUE(info->arguments()->GetString(0, &object_id));

  scoped_ptr<BindingObject> obj(new SysAppsTestObject);
  store_.AddBindingObject(object_id, obj.Pass());
}

void SysAppsTestExtensionInstance::OnHasObject(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  std::string object_id;
  ASSERT_TRUE(info->arguments()->GetString(0, &object_id));

  scoped_ptr<base::ListValue> result(new base::ListValue());
  result->AppendBoolean(store_.HasObjectForTesting(object_id));

  info->PostResult(result.Pass());
}

SysAppsTestObject::SysAppsTestObject() : is_test_event_active_(false) {
  handler_.Register("isTestEventActive",
      base::Bind(&SysAppsTestObject::OnIsTestEventActive,
                 base::Unretained(this)));
  handler_.Register("fireTestEvent",
      base::Bind(&SysAppsTestObject::OnFireTestEvent,
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
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<base::ListValue> result(new base::ListValue());
  result->AppendBoolean(is_test_event_active_);

  info->PostResult(result.Pass());
}

void SysAppsTestObject::OnFireTestEvent(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<base::ListValue> data(new base::ListValue());
  data->AppendString("Lorem ipsum");
  DispatchEvent("test", data.Pass());

  scoped_ptr<base::ListValue> result(new base::ListValue());
  info->PostResult(result.Pass());
}

class SysAppsCommonTest : public InProcessBrowserTest {
 public:
  void SetUp() {
    XWalkExtensionService::SetRegisterUIThreadExtensionsCallbackForTesting(
        base::Bind(&SysAppsCommonTest::RegisterExtensions,
                   base::Unretained(this)));
    InProcessBrowserTest::SetUp();
  }

  void RegisterExtensions(XWalkExtensionServer* server) {
    bool registered = server->RegisterExtension(
        scoped_ptr<XWalkExtension>(new SysAppsTestExtension()));
    ASSERT_TRUE(registered);
  }
};

IN_PROC_BROWSER_TEST_F(SysAppsCommonTest, SysAppsCommon) {
  const string16 passString = ASCIIToUTF16("Pass");
  const string16 failString = ASCIIToUTF16("Fail");

  content::RunAllPendingInMessageLoop();
  content::TitleWatcher title_watcher(runtime()->web_contents(), passString);
  title_watcher.AlsoWaitForTitle(failString);

  base::FilePath test_file;
  PathService::Get(base::DIR_SOURCE_ROOT, &test_file);
  test_file = test_file
      .Append(FILE_PATH_LITERAL("xwalk"))
      .Append(FILE_PATH_LITERAL("sysapps"))
      .Append(FILE_PATH_LITERAL("common"))
      .Append(FILE_PATH_LITERAL("common_api_browsertest.html"));

  xwalk_test_utils::NavigateToURL(runtime(), net::FilePathToFileURL(test_file));
  EXPECT_EQ(passString, title_watcher.WaitAndGetTitle());
}
