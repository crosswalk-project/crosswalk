// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_browser_main.h"

#include <iostream>

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/strings/sys_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/utf_string_conversions.h"
#include "content/public/browser/browser_main_runner.h"
#include "cameo/src/browser/shell.h"
#include "cameo/src/common/shell_switches.h"
#include "net/base/net_util.h"
#include "webkit/support/webkit_support.h"

namespace {

GURL GetURLForLayoutTest(const std::string& test_name,
                         base::FilePath* current_working_directory,
                         bool* enable_pixel_dumping,
                         std::string* expected_pixel_hash) {
  // A test name is formated like file:///path/to/test'--pixel-test'pixelhash
  std::string path_or_url = test_name;
  std::string pixel_switch;
  std::string pixel_hash;
  std::string::size_type separator_position = path_or_url.find('\'');
  if (separator_position != std::string::npos) {
    pixel_switch = path_or_url.substr(separator_position + 1);
    path_or_url.erase(separator_position);
  }
  separator_position = pixel_switch.find('\'');
  if (separator_position != std::string::npos) {
    pixel_hash = pixel_switch.substr(separator_position + 1);
    pixel_switch.erase(separator_position);
  }
  if (enable_pixel_dumping) {
    *enable_pixel_dumping =
        (pixel_switch == "--pixel-test" || pixel_switch == "-p");
  }
  if (expected_pixel_hash)
    *expected_pixel_hash = pixel_hash;
  GURL test_url(path_or_url);
  if (!(test_url.is_valid() && test_url.has_scheme())) {
    // We're outside of the message loop here, and this is a test.
    base::ThreadRestrictions::ScopedAllowIO allow_io;
#if defined(OS_WIN)
    std::wstring wide_path_or_url =
        base::SysNativeMBToWide(path_or_url);
    base::FilePath local_file(wide_path_or_url);
#else
    base::FilePath local_file(path_or_url);
#endif
    file_util::AbsolutePath(&local_file);
    test_url = net::FilePathToFileURL(local_file);
  }
  base::FilePath local_path;
  if (current_working_directory) {
    // We're outside of the message loop here, and this is a test.
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    if (net::FileURLToFilePath(test_url, &local_path))
      *current_working_directory = local_path.DirName();
    else
      file_util::GetCurrentDirectory(current_working_directory);
  }
  return test_url;
}

bool GetNextTest(const CommandLine::StringVector& args,
                 size_t* position,
                 std::string* test) {
  if (*position >= args.size())
    return false;
  if (args[*position] == FILE_PATH_LITERAL("-"))
    return !!std::getline(std::cin, *test, '\n');
#if defined(OS_WIN)
  *test = WideToUTF8(args[(*position)++]);
#else
  *test = args[(*position)++];
#endif
  return true;
}

}  // namespace

// Main routine for running as the Browser process.
int ShellBrowserMain(const content::MainFunctionParams& parameters) {
  scoped_ptr<content::BrowserMainRunner> main_runner_(
      content::BrowserMainRunner::Create());

  int exit_code = main_runner_->Initialize(parameters);

  if (exit_code >= 0)
    return exit_code;

  exit_code = main_runner_->Run();

  main_runner_->Shutdown();

  return exit_code;
}
