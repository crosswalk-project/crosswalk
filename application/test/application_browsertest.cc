// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "xwalk/application/test/application_browsertest.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"

namespace {

bool WaitForRuntimeCountCallback(int* count) {
  --(*count);
  return *count == 0;
}

}  // namespace

ApplicationBrowserTest::ApplicationBrowserTest() {
}

void ApplicationBrowserTest::SetUpCommandLine(CommandLine* commond_line) {
  PathService::Get(base::DIR_SOURCE_ROOT, &test_data_dir_);
  test_data_dir_ = test_data_dir_
    .Append(FILE_PATH_LITERAL("xwalk"))
    .Append(FILE_PATH_LITERAL("application"))
    .Append(FILE_PATH_LITERAL("test"))
    .Append(FILE_PATH_LITERAL("data"));
}

void ApplicationBrowserTest::WaitForRuntimes(int runtime_count) {
  int count = runtime_count - GetRuntimeNumber();
  if (count > 0) {
    content::WindowedNotificationObserver(
        xwalk::NOTIFICATION_RUNTIME_OPENED,
        base::Bind(&WaitForRuntimeCountCallback, &count)).Wait();
  } else if (count < 0) {
    count = -count;
    content::WindowedNotificationObserver(
        xwalk::NOTIFICATION_RUNTIME_CLOSED,
        base::Bind(&WaitForRuntimeCountCallback, &count)).Wait();
  }

  ASSERT_EQ(GetRuntimeNumber(), runtime_count);
}

int ApplicationBrowserTest::GetRuntimeNumber() {
  return xwalk::RuntimeRegistry::Get()->runtimes().size();
}
