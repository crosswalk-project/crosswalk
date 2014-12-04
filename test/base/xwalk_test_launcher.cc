// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/memory/linked_ptr.h"
#include "base/process/launch.h"
#include "base/run_loop.h"
#include "base/strings/string_util.h"
#include "base/sys_info.h"
#include "base/test/test_file_util.h"
#include "content/public/app/content_main.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "content/public/test/test_launcher.h"
#include "xwalk/runtime/app/xwalk_main_delegate.h"
#include "xwalk/test/base/xwalk_test_suite.h"

class XWalkTestLauncherDelegate : public content::TestLauncherDelegate {
 public:
  XWalkTestLauncherDelegate() {}
  virtual ~XWalkTestLauncherDelegate() {}

  int RunTestSuite(int argc, char** argv) override {
    return XWalkTestSuite(argc, argv).Run();
  }

  bool AdjustChildProcessCommandLine(
      CommandLine* command_line, const base::FilePath& temp_data_dir) override {
    CommandLine new_command_line(command_line->GetProgram());
    CommandLine::SwitchMap switches = command_line->GetSwitches();

    for (CommandLine::SwitchMap::const_iterator iter = switches.begin();
         iter != switches.end(); ++iter) {
      new_command_line.AppendSwitchNative((*iter).first, (*iter).second);
    }

    // Expose the garbage collector interface, so we can test the object
    // lifecycle tracker interface.
    new_command_line.AppendSwitchASCII(
        switches::kJavaScriptFlags, "--expose-gc");

    *command_line = new_command_line;
    return true;
  }

 protected:
  content::ContentMainDelegate* CreateContentMainDelegate() override {
#if defined(OS_WIN) || defined (OS_LINUX)
    return new xwalk::XWalkMainDelegate();
#else
    // This delegate is only guaranteed to link on linux and windows, so just
    // bail out if we are on any other platform.
    NOTREACHED();
    return NULL;
#endif
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkTestLauncherDelegate);
};

int main(int argc, char** argv) {
  int default_jobs = std::max(1, base::SysInfo::NumberOfProcessors() / 2);
  XWalkTestLauncherDelegate launcher_delegate;
  return content::LaunchTests(&launcher_delegate, default_jobs, argc, argv);
}
