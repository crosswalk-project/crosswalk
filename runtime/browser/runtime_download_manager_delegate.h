// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_RUNTIME_DOWNLOAD_MANAGER_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_RUNTIME_DOWNLOAD_MANAGER_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/download_manager_delegate.h"
#include "xwalk/runtime/browser/runtime.h"

namespace xwalk {

class RuntimeDownloadManagerDelegate
    : public content::DownloadManagerDelegate,
      public base::RefCountedThreadSafe<RuntimeDownloadManagerDelegate> {
 public:
  RuntimeDownloadManagerDelegate();

  void SetDownloadManager(content::DownloadManager* manager);

  void Shutdown() override;
  bool DetermineDownloadTarget(
      content::DownloadItem* download,
      const content::DownloadTargetCallback& callback) override;
  bool ShouldOpenDownload(
      content::DownloadItem* item,
      const content::DownloadOpenDelayedCallback& callback) override;
  void GetNextId(const content::DownloadIdCallback& callback) override;

  // Inhibits prompting and sets the default download path.
  void SetDownloadBehaviorForTesting(
      const base::FilePath& default_download_path);

 protected:
  // To allow subclasses for testing.
  ~RuntimeDownloadManagerDelegate() override;

 private:
  friend class base::RefCountedThreadSafe<RuntimeDownloadManagerDelegate>;

  void GenerateFilename(uint32 download_id,
                        const content::DownloadTargetCallback& callback,
                        const base::FilePath& generated_name,
                        const base::FilePath& suggested_directory);
  void OnDownloadPathGenerated(uint32 download_id,
                               const content::DownloadTargetCallback& callback,
                               const base::FilePath& suggested_path);
  void ChooseDownloadPath(uint32 download_id,
                          const content::DownloadTargetCallback& callback,
                          const base::FilePath& suggested_path);

  content::DownloadManager* download_manager_;
  base::FilePath default_download_path_;
  bool suppress_prompting_;

  DISALLOW_COPY_AND_ASSIGN(RuntimeDownloadManagerDelegate);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_RUNTIME_DOWNLOAD_MANAGER_DELEGATE_H_
