// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/internal_extension_browsertest.h"

#include <algorithm>
#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "grit/xwalk_extensions_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/extensions/test/test.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using namespace xwalk::extensions; // NOLINT
using namespace xwalk::jsapi::test; // NOLINT
using xwalk::Runtime;

TestExtension::TestExtension() {
  set_name("test");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_EXTENSIONS_TESTS_INTERNAL_EXTENSION_BROWSERTEST_API)
                     .as_string());
}

XWalkExtensionInstance* TestExtension::CreateInstance() {
  return new TestExtensionInstance();
}

TestExtensionInstance::TestExtensionInstance() : counter_(0), handler_(this) {
  handler_.Register("clearDatabase",
      base::Bind(&TestExtensionInstance::OnClearDatabase,
                 base::Unretained(this)));
  handler_.Register("addPerson",
      base::Bind(&TestExtensionInstance::OnAddPerson,
                 base::Unretained(this)));
  handler_.Register("addPersonObject",
      base::Bind(&TestExtensionInstance::OnAddPersonObject,
                 base::Unretained(this)));
  handler_.Register("getAllPersons",
      base::Bind(&TestExtensionInstance::OnGetAllPersons,
                 base::Unretained(this)));
  handler_.Register("getPersonAge",
      base::Bind(&TestExtensionInstance::OnGetPersonAge,
                 base::Unretained(this)));
  handler_.Register("startHeartbeat",
      base::Bind(&TestExtensionInstance::OnStartHeartbeat,
                 base::Unretained(this)));
  handler_.Register("stopHeartbeat",
      base::Bind(&TestExtensionInstance::OnStopHeartbeat,
                 base::Unretained(this)));
}

TestExtensionInstance::~TestExtensionInstance() {}

void TestExtensionInstance::HandleMessage(std::unique_ptr<base::Value> msg) {
  handler_.HandleMessage(std::move(msg));
}

void TestExtensionInstance::OnClearDatabase(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  database()->clear();
}

void TestExtensionInstance::OnAddPerson(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<AddPerson::Params>
      params(AddPerson::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  std::pair<std::string, int> person(params->name, params->age);
  database()->push_back(person);
}

void TestExtensionInstance::OnAddPersonObject(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<AddPersonObject::Params>
      params(AddPersonObject::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  std::pair<std::string, int> person(params->person.name, params->person.age);
  database()->push_back(person);
}

void TestExtensionInstance::OnGetAllPersons(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<GetAllPersons::Params>
      params(GetAllPersons::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  size_t max_size = std::min<size_t>(database()->size(), params->max_size);
  std::vector<Person> persons;

  for (size_t i = 0; i < max_size; ++i) {
    Person person;
    person.name = database()->at(i).first;
    person.age = database()->at(i).second;

    persons.push_back(std::move(person));
  }

  info->PostResult(
    GetAllPersons::Results::Create(persons, base::checked_cast<int>(max_size)));
}

void TestExtensionInstance::OnGetPersonAge(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  std::unique_ptr<GetPersonAge::Params>
      params(GetPersonAge::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  int age = -1;

  for (unsigned i = 0; i < database()->size(); ++i) {
    if (database()->at(i).first == params->name)
      age = database()->at(i).second;
  }

  info->PostResult(GetPersonAge::Results::Create(age));
}

void TestExtensionInstance::OnStartHeartbeat(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  heartbeat_info_.reset(info.release());
  timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(50),
               this,
               &TestExtensionInstance::DispatchHeartbeat);
}

void TestExtensionInstance::OnStopHeartbeat(
    std::unique_ptr<XWalkExtensionFunctionInfo> info) {
  timer_.Stop();
}

void TestExtensionInstance::DispatchHeartbeat() {
  heartbeat_info_->PostResult(StartHeartbeat::Results::Create(counter_++));
}

class InternalExtensionTest : public XWalkExtensionsTestBase {
 public:
  void CreateExtensionsForUIThread(
      XWalkExtensionVector* extensions) override {
    extensions->push_back(new TestExtension);
  }
};

IN_PROC_BROWSER_TEST_F(InternalExtensionTest, InternalExtension) {
  Runtime* runtime = CreateRuntime();
  content::TitleWatcher title_watcher(runtime->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("test_internal_extension.html"));
  xwalk_test_utils::NavigateToURL(runtime, url);

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
