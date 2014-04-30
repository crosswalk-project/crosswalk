// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_platform_util.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/process/kill.h"
#include "base/process/launch.h"
#include "url/gurl.h"

namespace platform_util {
namespace {
// The system default web browser path.
// In some Tizen releases, there exists a system browser called 'MiniBrowser',
// which we can use to open an external link from a web app.
const char kWebBrowserPath[] = "/usr/bin/MiniBrowser";
}  // namespace

void OpenExternal(const GURL& url) {
  if (url.SchemeIsHTTPOrHTTPS()) {
    LOG(INFO) << "Open in WebBrowser.";
    std::vector<std::string> argv;
    if (base::PathExists(base::FilePath(kWebBrowserPath)))
      argv.push_back(kWebBrowserPath);
    else
      argv.push_back("xwalk");
    argv.push_back(url.spec());
    base::ProcessHandle handle;

    if (base::LaunchProcess(argv, base::LaunchOptions(), &handle))
      base::EnsureProcessGetsReaped(handle);
  }
}

}  // namespace platform_util
