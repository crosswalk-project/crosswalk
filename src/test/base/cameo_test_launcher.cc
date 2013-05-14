// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_launcher.h"

#include <stack>

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/linked_ptr.h"
#include "base/process_util.h"
#include "base/run_loop.h"
#include "base/string_util.h"
#include "base/test/test_file_util.h"
#include "cameo/src/runtime/app/cameo_main_delegate.h"
#include "cameo/src/test/base/cameo_test_suite.h"
#include "content/public/app/content_main.h"
#include "content/public/browser/browser_thread.h"

#if defined(OS_WIN)
#include "content/public/app/startup_helper_win.h"
#include "sandbox/win/src/sandbox_types.h"
#endif  // defined(OS_WIN)

#if defined(TOOLKIT_VIEWS)
#include "ui/views/focus/accelerator_handler.h"
#endif

const char kEmptyTestName[] = "InProcessBrowserTest.Empty";

class CameoTestLauncherDelegate : public content::TestLauncherDelegate {
 public:
  CameoTestLauncherDelegate() {}
  virtual ~CameoTestLauncherDelegate() {}

  virtual std::string GetEmptyTestName() OVERRIDE {
    return kEmptyTestName;
  }

  virtual int RunTestSuite(int argc, char** argv) OVERRIDE {
    return CameoTestSuite(argc, argv).Run();
  }

  virtual bool AdjustChildProcessCommandLine(
      CommandLine* command_line, const base::FilePath& temp_data_dir) OVERRIDE {
    CommandLine new_command_line(command_line->GetProgram());
    CommandLine::SwitchMap switches = command_line->GetSwitches();

    for (CommandLine::SwitchMap::const_iterator iter = switches.begin();
         iter != switches.end(); ++iter) {
      new_command_line.AppendSwitchNative((*iter).first, (*iter).second);
    }

    *command_line = new_command_line;
    return true;
  }

  virtual void PreRunMessageLoop(base::RunLoop* run_loop) OVERRIDE {
#if defined(TOOLKIT_VIEWS)
    if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
      linked_ptr<views::AcceleratorHandler> handler(
          new views::AcceleratorHandler);
      handlers_.push(handler);
      run_loop->set_dispatcher(handler.get());
    }
#endif
  }

  virtual void PostRunMessageLoop() OVERRIDE {
#if defined(TOOLKIT_VIEWS)
    if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
      DCHECK_EQ(handlers_.empty(), false);
      handlers_.pop();
    }
#endif
  }

 protected:
  virtual content::ContentMainDelegate* CreateContentMainDelegate() OVERRIDE {
#if defined(OS_WIN) || defined (OS_LINUX)
    return new cameo::CameoMainDelegate();
#else
    // This delegate is only guaranteed to link on linux and windows, so just
    // bail out if we are on any other platform.
    NOTREACHED();
    return NULL;
#endif
  }

 private:
#if defined(TOOLKIT_VIEWS)
  std::stack<linked_ptr<views::AcceleratorHandler> > handlers_;
#endif

  DISALLOW_COPY_AND_ASSIGN(CameoTestLauncherDelegate);
};

int main(int argc, char** argv) {
  CameoTestLauncherDelegate launcher_delegate;
  return content::LaunchTests(&launcher_delegate, argc, argv);
}
