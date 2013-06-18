// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/test/base/cameo_test_utils.h"

#include <set>

#include "base/command_line.h"
#include "base/environment.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/string_number_conversions.h"
#include "cameo/src/runtime/browser/runtime.h"
#include "cameo/src/runtime/common/cameo_paths.h"
#include "cameo/src/runtime/common/cameo_switches.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/result_codes.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "ui/gl/gl_switches.h"

using content::NavigationController;
using content::TestNavigationObserver;
using content::WebContents;

namespace cameo_test_utils {

void PrepareBrowserCommandLineForTests(CommandLine* command_line) {
  // Enable info level logging by default so that we can see when bad
  // stuff happens, but honor the flags specified from the command line.
  if (!command_line->HasSwitch(switches::kEnableLogging))
    command_line->AppendSwitch(switches::kEnableLogging);
  if (!command_line->HasSwitch(switches::kLoggingLevel))
    command_line->AppendSwitchASCII(switches::kLoggingLevel, "0");

  // Don't collect GPU info, load GPU blacklist, or schedule a GPU blacklist
  // auto-update.
  command_line->AppendSwitch(switches::kSkipGpuDataLoading);
}

bool OverrideDataPathDir(const base::FilePath& data_path_dir) {
  // PathService::Override() is the best way to change the data path directory.
  return PathService::Override(cameo::DIR_DATA_PATH, data_path_dir);
}

base::FilePath GetTestFilePath(const base::FilePath& dir,
                               const base::FilePath& file) {
  base::FilePath test_base_dir;
  PathService::Get(cameo::DIR_TEST_DATA, &test_base_dir);

  return test_base_dir.Append(dir).Append(file);
}

GURL GetTestURL(const base::FilePath& dir, const base::FilePath& file) {
  return net::FilePathToFileURL(GetTestFilePath(dir, file));
}

// Navigate a specified URL in the given Runtime. It will block until the
// navigation completes.
void NavigateToURL(cameo::Runtime* runtime, const GURL& url) {
  if (runtime->web_contents()->IsLoading())
    content::WaitForLoadStop(runtime->web_contents());

  NavigationController& controller = runtime->web_contents()->GetController();
  TestNavigationObserver navigation_observer(
      content::Source<NavigationController>(&controller), 1);
  runtime->LoadURL(url);

  base::RunLoop run_loop;
  navigation_observer.WaitForObservation(
      base::Bind(&content::RunThisRunLoop, base::Unretained(&run_loop)),
      content::GetQuitTaskForRunLoop(&run_loop));
}

class ChildProcessFilter : public base::ProcessFilter {
 public:
  explicit ChildProcessFilter(base::ProcessId parent_pid)
      : parent_pids_(&parent_pid, (&parent_pid) + 1) {}

  explicit ChildProcessFilter(const std::vector<base::ProcessId>& parent_pids)
      : parent_pids_(parent_pids.begin(), parent_pids.end()) {}

  virtual bool Includes(const base::ProcessEntry& entry) const OVERRIDE {
    return parent_pids_.find(entry.parent_pid()) != parent_pids_.end();
  }

 private:
  const std::set<base::ProcessId> parent_pids_;

  DISALLOW_COPY_AND_ASSIGN(ChildProcessFilter);
};

const base::FilePath::StringType GetRunningRuntimeExecutableName() {
  const CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  return cmd_line->GetProgram().BaseName().value();
}

CameoProcessList GetRunningCameoProcesses(base::ProcessId runtime_pid) {
  const base::FilePath::StringType executable_name =
      GetRunningRuntimeExecutableName();
  CameoProcessList result;
  if (runtime_pid == static_cast<base::ProcessId>(-1))
    return result;

  ChildProcessFilter filter(runtime_pid);
  base::NamedProcessIterator it(executable_name, &filter);
  while (const base::ProcessEntry* process_entry = it.NextProcessEntry()) {
    result.push_back(process_entry->pid());
  }

#if defined(OS_POSIX) && !defined(OS_MACOSX)
  // On Unix we might be running with a zygote process for the renderers.
  // Because of that we sweep the list of processes again and pick those which
  // are children of one of the processes that we've already seen.
  {
    ChildProcessFilter filter(result);
    base::NamedProcessIterator it(executable_name, &filter);
    while (const base::ProcessEntry* process_entry = it.NextProcessEntry())
      result.push_back(process_entry->pid());
  }
#endif  // defined(OS_POSIX) && !defined(OS_MACOSX)

  return result;
}

void TerminateAllCameoProcesses(const CameoProcessList& process_pids) {
  CameoProcessList::const_iterator it;
  for (it = process_pids.begin(); it != process_pids.end(); ++it) {
    DLOG(INFO) << "Killing child processes: " << *it;
    base::ProcessHandle handle;
    if (!base::OpenProcessHandle(*it, &handle)) {
      // Ignore processes for which we can't open the handle. We don't
      // guarantee that all processes will terminate, only try to do so.
      continue;
    }

    base::KillProcess(handle, content::RESULT_CODE_KILLED, true);
    base::CloseProcessHandle(handle);
  }
}

}  // namespace cameo_test_utils
