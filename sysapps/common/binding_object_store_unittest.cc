// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/sysapps/common/binding_object_store.h"

#include "xwalk/extensions/browser/xwalk_extension_function_handler.h"
#include "testing/gtest/include/gtest/gtest.h"

using xwalk::extensions::XWalkExtensionFunctionHandler;
using xwalk::extensions::XWalkExtensionFunctionInfo;
using xwalk::sysapps::BindingObject;
using xwalk::sysapps::BindingObjectStore;

namespace {

const char kTestString[] = "crosswalk1234567890";

void DummyCallback(scoped_ptr<base::ListValue> result) {}

scoped_ptr<XWalkExtensionFunctionInfo> CreateFunctionInfo(
    const std::string& name, const std::string& str_argument) {
  scoped_ptr<base::ListValue> arguments(new base::ListValue);
  arguments->AppendString(str_argument);

  return make_scoped_ptr(new XWalkExtensionFunctionInfo(
      name,
      arguments.Pass(),
      base::Bind(&DummyCallback)));
}

class BindingObjectTest : public BindingObject {
 public:
  static scoped_ptr<BindingObject> Create() {
    return make_scoped_ptr(new BindingObjectTest()).PassAs<BindingObject>();
  }

  BindingObjectTest() : call_count_(0) {
    instance_count_++;

    handler_.Register("test",
        base::Bind(&BindingObjectTest::OnTest, base::Unretained(this)));
  }

  virtual ~BindingObjectTest() {
    instance_count_--;
  }

  static int instance_count() {
    return instance_count_;
  }

  int call_count() const {
    return call_count_;
  }

 private:
  void OnTest(scoped_ptr<XWalkExtensionFunctionInfo> info) {
    EXPECT_EQ(info->name(), "test");

    std::string test_string;
    info->arguments()->GetString(0, &test_string);
    EXPECT_EQ(test_string, kTestString);

    call_count_++;
  }

  int call_count_;
  static int instance_count_;
};

int BindingObjectTest::instance_count_ = 0;

}  // namespace

TEST(XWalkSysAppsBindingObjectStoreTest, AddBindingObject) {
  scoped_ptr<XWalkExtensionFunctionHandler> handler(
      new XWalkExtensionFunctionHandler(NULL));
  scoped_ptr<BindingObjectStore> store(new BindingObjectStore(handler.get()));

  EXPECT_FALSE(store->HasObjectForTesting("foobar1"));
  EXPECT_FALSE(store->HasObjectForTesting("foobar2"));
  EXPECT_FALSE(store->HasObjectForTesting("foobar3"));
  EXPECT_FALSE(store->HasObjectForTesting("foobar4"));

  store->AddBindingObject("foobar1", BindingObjectTest::Create());
  store->AddBindingObject("foobar2", BindingObjectTest::Create());
  store->AddBindingObject("foobar3", BindingObjectTest::Create());
  store->AddBindingObject("foobar4", BindingObjectTest::Create());

  EXPECT_TRUE(store->HasObjectForTesting("foobar1"));
  EXPECT_TRUE(store->HasObjectForTesting("foobar2"));
  EXPECT_TRUE(store->HasObjectForTesting("foobar3"));
  EXPECT_TRUE(store->HasObjectForTesting("foobar4"));

  EXPECT_EQ(BindingObjectTest::instance_count(), 4);

  handler.reset(new XWalkExtensionFunctionHandler(NULL));
  store.reset(new BindingObjectStore(handler.get()));
  EXPECT_EQ(BindingObjectTest::instance_count(), 0);

  // Same ID, should discard the object. If this is happening in
  // real life, there is something wrong with the code (and that is
  // why we print a warning).
  store->AddBindingObject("foobar1", BindingObjectTest::Create());
  store->AddBindingObject("foobar1", BindingObjectTest::Create());
  store->AddBindingObject("foobar1", BindingObjectTest::Create());
  store->AddBindingObject("foobar1", BindingObjectTest::Create());
  EXPECT_EQ(BindingObjectTest::instance_count(), 1);

  store.reset();
  EXPECT_EQ(BindingObjectTest::instance_count(), 0);
}

TEST(XWalkSysAppsBindingObjectStoreTest, OnJSObjectCollected) {
  XWalkExtensionFunctionHandler handler(NULL);
  scoped_ptr<BindingObjectStore> store(new BindingObjectStore(&handler));

  store->AddBindingObject("foobar1", BindingObjectTest::Create());
  store->AddBindingObject("foobar2", BindingObjectTest::Create());
  store->AddBindingObject("foobar3", BindingObjectTest::Create());
  store->AddBindingObject("foobar4", BindingObjectTest::Create());
  EXPECT_EQ(BindingObjectTest::instance_count(), 4);

  EXPECT_TRUE(handler.HandleFunction(
          CreateFunctionInfo("JSObjectCollected", "foobar1")));
  EXPECT_EQ(BindingObjectTest::instance_count(), 3);

  EXPECT_TRUE(handler.HandleFunction(
          CreateFunctionInfo("JSObjectCollected", "foobar2")));
  EXPECT_EQ(BindingObjectTest::instance_count(), 2);

  // Attempt to destroy an object that doesn't exist
  // on the store.
  EXPECT_TRUE(handler.HandleFunction(
          CreateFunctionInfo("JSObjectCollected", "foobar2")));
  EXPECT_EQ(BindingObjectTest::instance_count(), 2);

  store.reset();
  EXPECT_EQ(BindingObjectTest::instance_count(), 0);
}

TEST(XWalkSysAppsBindingObjectStoreTest, OnPostMessageToObject) {
  XWalkExtensionFunctionHandler handler(NULL);
  scoped_ptr<BindingObjectStore> store(new BindingObjectStore(&handler));

  BindingObjectTest* binding_object1(new BindingObjectTest());
  BindingObjectTest* binding_object2(new BindingObjectTest());
  scoped_ptr<BindingObject> binding_object_ptr1(binding_object1);
  scoped_ptr<BindingObject> binding_object_ptr2(binding_object2);

  store->AddBindingObject("foobar1", binding_object_ptr1.Pass());
  store->AddBindingObject("foobar2", binding_object_ptr2.Pass());
  EXPECT_EQ(BindingObjectTest::instance_count(), 2);

  for (unsigned i = 0; i < 1000; ++i) {
    scoped_ptr<base::ListValue> arguments(new base::ListValue);

    // Object ID.
    arguments->AppendString("foobar1");

    // Function name on the target object.
    arguments->AppendString("test");

    // Arguments list passed to the target object.
    base::ListValue* targetArguments(new base::ListValue());
    targetArguments->AppendString(kTestString);
    arguments->Append(targetArguments);

    scoped_ptr<XWalkExtensionFunctionInfo> postMessageToObjectInfo(
        new XWalkExtensionFunctionInfo(
            "postMessageToObject",
            arguments.Pass(),
            base::Bind(&DummyCallback)));

    EXPECT_TRUE(handler.HandleFunction(postMessageToObjectInfo.Pass()));
    EXPECT_EQ(binding_object1->call_count(), i + 1);
  }

  EXPECT_EQ(binding_object2->call_count(), 0);

  store.reset();
  EXPECT_EQ(BindingObjectTest::instance_count(), 0);
}
