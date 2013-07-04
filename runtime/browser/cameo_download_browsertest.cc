// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "cameo/runtime/browser/runtime.h"
#include "cameo/runtime/browser/runtime_download_manager_delegate.h"
#include "cameo/runtime/browser/ui/color_chooser.h"
#include "cameo/test/base/cameo_test_utils.h"
#include "cameo/test/base/in_process_browser_test.h"
#include "content/browser/download/download_manager_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/download_test_observer.h"
#include "content/public/test/test_utils.h"

using cameo::Runtime;
using cameo::RuntimeDownloadManagerDelegate;
using content::DownloadItem;
using content::DownloadManager;
using content::DownloadManagerImpl;
using content::DownloadTestObserver;
using content::DownloadTestObserverTerminal;
using content::BrowserContext;

namespace {

static DownloadManagerImpl* DownloadManagerForCameo(Runtime* runtime) {
  return static_cast<DownloadManagerImpl*>(
      BrowserContext::GetDownloadManager(
          runtime->web_contents()->GetBrowserContext()));
}

class CameoDownloadBroswerTest : public InProcessBrowserTest {
 public:
  virtual void SetUpOnMainThread() OVERRIDE {
    ASSERT_TRUE(downloads_directory_.CreateUniqueTempDir());

    DownloadManagerImpl* manager = DownloadManagerForCameo(runtime());
    RuntimeDownloadManagerDelegate* delegate =
        static_cast<RuntimeDownloadManagerDelegate*>(manager->GetDelegate());
    delegate->SetDownloadBehaviorForTesting(downloads_directory_.path());
  }

  // Create a DownloadTestObserverTerminal that will wait for the
  // specified number of downloads to finish.
  DownloadTestObserver* CreateWaiter(Runtime* runtime, int num_downloads) {
    DownloadManager* download_manager = DownloadManagerForCameo(runtime);
    return new DownloadTestObserverTerminal(download_manager, num_downloads,
        DownloadTestObserver::ON_DANGEROUS_DOWNLOAD_FAIL);
  }

 private:
  // Location of the downloads directory for these tests
  base::ScopedTempDir downloads_directory_;
};

IN_PROC_BROWSER_TEST_F(CameoDownloadBroswerTest, FileDownload) {
  GURL url = cameo_test_utils::GetTestURL(
      base::FilePath().AppendASCII("download"),
      base::FilePath().AppendASCII("test.lib"));
  scoped_ptr<DownloadTestObserver> observer(CreateWaiter(runtime(), 1));
  cameo_test_utils::NavigateToURL(runtime(), url);
  observer->WaitForFinished();
  EXPECT_EQ(1u, observer->NumDownloadsSeenInState(DownloadItem::COMPLETE));
  std::vector<DownloadItem*> downloads;
  DownloadManagerForCameo(runtime())->GetAllDownloads(&downloads);
  ASSERT_EQ(1u, downloads.size());
  ASSERT_EQ(DownloadItem::COMPLETE, downloads[0]->GetState());
  base::FilePath file(downloads[0]->GetFullPath());
  ASSERT_TRUE(file_util::ContentsEqual(
      file, cameo_test_utils::GetTestFilePath(
          base::FilePath().AppendASCII("download"),
          base::FilePath().AppendASCII("test.lib"))));
}

}  // namespace
