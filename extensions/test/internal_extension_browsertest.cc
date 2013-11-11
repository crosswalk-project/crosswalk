// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/internal_extension_browsertest.h"

#include <algorithm>
#include "base/logging.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/grit/xwalk_extensions_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/extensions/test/test.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

using namespace xwalk::extensions; // NOLINT
using namespace xwalk::jsapi::test; // NOLINT

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

void TestExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void TestExtensionInstance::OnClearDatabase(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  database()->clear();
}

void TestExtensionInstance::OnAddPerson(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<AddPerson::Params>
      params(AddPerson::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  std::pair<std::string, int> person(params->name, params->age);
  database()->push_back(person);
}

void TestExtensionInstance::OnAddPersonObject(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<AddPersonObject::Params>
      params(AddPersonObject::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  std::pair<std::string, int> person(params->person.name, params->person.age);
  database()->push_back(person);
}

void TestExtensionInstance::OnGetAllPersons(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<GetAllPersons::Params>
      params(GetAllPersons::Params::Create(*info->arguments()));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << info->name();
    return;
  }

  unsigned max_size = std::min<unsigned>(database()->size(), params->max_size);
  std::vector<linked_ptr<Person> > persons;

  for (unsigned i = 0; i < max_size; ++i) {
    linked_ptr<Person> person(new Person);
    person->name = database()->at(i).first;
    person->age = database()->at(i).second;

    persons.push_back(person);
  }

  info->PostResult(GetAllPersons::Results::Create(persons, max_size));
}

void TestExtensionInstance::OnGetPersonAge(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  scoped_ptr<GetPersonAge::Params>
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
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  heartbeat_info_.reset(info.release());
  timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(50),
               this,
               &TestExtensionInstance::DispatchHeartbeat);
}

void TestExtensionInstance::OnStopHeartbeat(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  timer_.Stop();
}

void TestExtensionInstance::DispatchHeartbeat() {
  heartbeat_info_->PostResult(StartHeartbeat::Results::Create(counter_++));
}

class InternalExtensionTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionServer* server) OVERRIDE {
    ASSERT_TRUE(RegisterExtensionForTest(server, new TestExtension));
  }
};

IN_PROC_BROWSER_TEST_F(InternalExtensionTest, InternalExtension) {
  content::RunAllPendingInMessageLoop();

  content::TitleWatcher title_watcher(runtime()->web_contents(), kPassString);
  title_watcher.AlsoWaitForTitle(kFailString);

  GURL url = GetExtensionsTestURL(base::FilePath(),
      base::FilePath().AppendASCII("test_internal_extension.html"));
  xwalk_test_utils::NavigateToURL(runtime(), url);

  EXPECT_EQ(kPassString, title_watcher.WaitAndGetTitle());
}
