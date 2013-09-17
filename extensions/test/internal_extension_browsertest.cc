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

TestExtensionInstance::TestExtensionInstance()
    : XWalkInternalExtensionInstance() {
  RegisterFunction("clearDatabase", &TestExtensionInstance::OnClearDatabase);
  RegisterFunction("addPerson", &TestExtensionInstance::OnAddPerson);
  RegisterFunction("addPersonObject",
      &TestExtensionInstance::OnAddPersonObject);
  RegisterFunction("getAllPersons", &TestExtensionInstance::OnGetAllPersons);
  RegisterFunction("getPersonAge", &TestExtensionInstance::OnGetPersonAge);
}

void TestExtensionInstance::OnClearDatabase(const std::string&,
                                                          const std::string&,
                                                          base::ListValue*) {
  database()->clear();
}

void TestExtensionInstance::OnAddPerson(
    const std::string& function_name, const std::string&,
    base::ListValue* args) {
  scoped_ptr<AddPerson::Params> params(AddPerson::Params::Create(*args));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << function_name;
    return;
  }

  std::pair<std::string, int> person(params->name, params->age);
  database()->push_back(person);
}

void TestExtensionInstance::OnAddPersonObject(
    const std::string& function_name, const std::string&,
    base::ListValue* args) {
  scoped_ptr<AddPersonObject::Params>
      params(AddPersonObject::Params::Create(*args));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << function_name;
    return;
  }

  std::pair<std::string, int> person(params->person.name, params->person.age);
  database()->push_back(person);
}

void TestExtensionInstance::OnGetAllPersons(
    const std::string& function_name, const std::string& callback_id,
    base::ListValue* args) {
  if (callback_id.empty())
    return;

  scoped_ptr<GetAllPersons::Params>
      params(GetAllPersons::Params::Create(*args));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << function_name;
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

  PostResult(callback_id,
             GetAllPersons::Results::Create(persons, max_size));
}

void TestExtensionInstance::OnGetPersonAge(
    const std::string& function_name, const std::string& callback_id,
    base::ListValue* args) {
  if (callback_id.empty())
    return;

  scoped_ptr<GetPersonAge::Params>
      params(GetPersonAge::Params::Create(*args));

  if (!params) {
    LOG(WARNING) << "Malformed parameters passed to " << function_name;
    return;
  }

  int age = -1;

  for (unsigned i = 0; i < database()->size(); ++i) {
    if (database()->at(i).first == params->name)
      age = database()->at(i).second;
  }

  PostResult(callback_id, GetPersonAge::Results::Create(age));
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
