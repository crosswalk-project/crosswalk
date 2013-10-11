// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/test/internal_extension_browsertest.h"

#include <algorithm>
#include "base/logging.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/test/test.h"
#include "xwalk/extensions/test/xwalk_extensions_test_base.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/test/base/xwalk_test_utils.h"

extern const char kSource_internal_extension_browsertest_api[];

using namespace xwalk::extensions; // NOLINT
using namespace xwalk::jsapi_test::test; // NOLINT

TestExtension::TestExtension() {
  set_name("test");
}

const char* TestExtension::GetJavaScriptAPI() {
  return kSource_internal_extension_browsertest_api;
}

XWalkExtensionInstance* TestExtension::CreateInstance() {
  return new TestExtensionInstance();
}

TestExtensionInstance::TestExtensionInstance() {
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
}

void TestExtensionInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass(), this);
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

class InternalExtensionTest : public XWalkExtensionsTestBase {
 public:
  void RegisterExtensions(XWalkExtensionService* extension_service) OVERRIDE {
    bool registered = extension_service->RegisterExtension(
        scoped_ptr<XWalkExtension>(new TestExtension()));
    ASSERT_TRUE(registered);
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
