// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"
#include "xwalk/test/base/in_process_browser_test.h"
#include "content/public/test/test_utils.h"

class XWalkSwitchesTest : public InProcessBrowserTest {
 public:
  virtual void SetUp() OVERRIDE {
    base::ScopedTempDir temp_dir;
    if (temp_dir.CreateUniqueTempDir() && temp_dir.IsValid()) {
      data_path_ = temp_dir.path();
    } else {
      LOG(ERROR) << "Failed to create temporary directory \""
                 << temp_dir.path().value() << "\"";
    }

    InProcessBrowserTest::SetUp();

    // The scoped |temp_dir| will be deleted after out of scope. We just want
    // to make use of the pathname of temporary directory as the Crosswalk data
    // path, and the Runtime instance will create its data path directory if
    // it doesn't exist.
  }

  virtual void TearDown() OVERRIDE {
    InProcessBrowserTest::TearDown();
    // Since Runtime instance creates its own data path directory
    // for testing, we need to clean up it finally.
    if (base::PathExists(data_path_))
      base::DeleteFile(data_path_, true);
  }

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    command_line->AppendSwitchPath(switches::kXWalkDataPath, data_path_);
  }

  base::FilePath GetDataPath() const { return data_path_; }

 private:
  base::FilePath data_path_;
};

IN_PROC_BROWSER_TEST_F(XWalkSwitchesTest, kXWalkDataPath) {
  content::RunAllPendingInMessageLoop();
  // The data path should be created by Runtime itself.
  base::FilePath data_path = GetDataPath();
  EXPECT_TRUE(base::PathExists(data_path));
  EXPECT_NE(base::FilePath(), data_path);

  // kXWalkDataPath option should also override the DIR_DATA_PATH value
  // registered in PathService.
  base::FilePath path;
  EXPECT_TRUE(PathService::Get(xwalk::DIR_DATA_PATH, &path));
  EXPECT_EQ(path, data_path);
}
