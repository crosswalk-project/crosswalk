// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/base/xwalk_test_suite.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/memory/ref_counted.h"
#include "base/metrics/stats_table.h"
#include "base/path_service.h"
#include "base/process/process_handle.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/common/xwalk_content_client.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "content/public/test/test_launcher.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/dns/mock_host_resolver.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_handle.h"

#if defined(OS_POSIX)
#include "base/memory/shared_memory.h"
#endif

namespace {

void RemoveSharedMemoryFile(const std::string& filename) {
  // Stats uses SharedMemory under the hood. On posix, this results in a file
  // on disk.
#if defined(OS_POSIX)
  base::SharedMemory memory;
  memory.Delete(filename);
#endif
}

class XWalkTestSuiteInitializer : public testing::EmptyTestEventListener {
 public:
  XWalkTestSuiteInitializer() {
  }

  virtual void OnTestStart(const testing::TestInfo& test_info) OVERRIDE {
    content_client_.reset(new xwalk::XWalkContentClient);
    content::SetContentClient(content_client_.get());

    browser_content_client_.reset(new xwalk::XWalkContentBrowserClient());
    SetBrowserClientForTesting(browser_content_client_.get());
  }

  virtual void OnTestEnd(const testing::TestInfo& test_info) OVERRIDE {
    browser_content_client_.reset();
    content_client_.reset();
    content::SetContentClient(NULL);
  }

 private:
  scoped_ptr<xwalk::XWalkContentClient> content_client_;
  scoped_ptr<xwalk::XWalkContentBrowserClient> browser_content_client_;

  DISALLOW_COPY_AND_ASSIGN(XWalkTestSuiteInitializer);
};

}  // namespace

XWalkTestSuite::XWalkTestSuite(int argc, char** argv)
    : content::ContentTestSuiteBase(argc, argv) {
}

XWalkTestSuite::~XWalkTestSuite() {
}

void XWalkTestSuite::Initialize() {
  xwalk::RegisterPathProvider();

  // Initialize after overriding paths as some content paths depend on correct
  // values for DIR_EXE and DIR_MODULE.
  content::ContentTestSuiteBase::Initialize();

  base::FilePath pak_dir;
  PathService::Get(base::DIR_MODULE, &pak_dir);
  DCHECK(!pak_dir.empty());
  base::FilePath resources_pack_path;
  resources_pack_path = pak_dir.Append(FILE_PATH_LITERAL("xwalk.pak"));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(resources_pack_path);

  stats_filename_ = base::StringPrintf("unit_tests-%d",
                                       base::GetCurrentProcId());
  RemoveSharedMemoryFile(stats_filename_);
  stats_table_.reset(new base::StatsTable(stats_filename_, 20, 200));
  base::StatsTable::set_current(stats_table_.get());

  testing::TestEventListeners& listeners =
      testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new XWalkTestSuiteInitializer);
}

content::ContentClient* XWalkTestSuite::CreateClientForInitialization() {
  return new xwalk::XWalkContentClient();
}

void XWalkTestSuite::Shutdown() {
  ResourceBundle::CleanupSharedInstance();

  base::StatsTable::set_current(NULL);
  stats_table_.reset();
  RemoveSharedMemoryFile(stats_filename_);

  base::TestSuite::Shutdown();
}
