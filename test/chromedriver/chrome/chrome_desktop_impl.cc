// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/chrome/chrome_desktop_impl.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/posix/eintr_wrapper.h"
#include "base/process/kill.h"
#include "base/sys_info.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "chrome/test/chromedriver/chrome/automation_extension.h"
#include "chrome/test/chromedriver/chrome/devtools_client.h"
#include "chrome/test/chromedriver/chrome/devtools_http_client.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/web_view_impl.h"
#include "chrome/test/chromedriver/net/port_server.h"

#if defined(OS_POSIX)
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

bool KillProcess(base::ProcessHandle process_id) {
#if defined(OS_POSIX)
  kill(process_id, SIGKILL);
  base::TimeTicks deadline =
      base::TimeTicks::Now() + base::TimeDelta::FromSeconds(5);
  while (base::TimeTicks::Now() < deadline) {
    pid_t pid = HANDLE_EINTR(waitpid(process_id, NULL, WNOHANG));
    if (pid == process_id)
      return true;
    if (pid == -1) {
      if (errno == ECHILD) {
        // The wait may fail with ECHILD if another process also waited for
        // the same pid, causing the process state to get cleaned up.
        return true;
      }
      LOG(WARNING) << "Error waiting for process " << process_id;
    }
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(50));
  }
  return false;
#endif

#if defined(OS_WIN)
  if (!base::KillProcess(process_id, 0, true)) {
    int exit_code;
    return base::GetTerminationStatus(process_id, &exit_code) !=
        base::TERMINATION_STATUS_STILL_RUNNING;
  }
  return true;
#endif
}

}  // namespace

ChromeDesktopImpl::ChromeDesktopImpl(
    scoped_ptr<DevToolsHttpClient> client,
    ScopedVector<DevToolsEventListener>& devtools_event_listeners,
    scoped_ptr<PortReservation> port_reservation,
    base::ProcessHandle process,
    const CommandLine& command,
    base::ScopedTempDir* user_data_dir,
    base::ScopedTempDir* extension_dir)
    : ChromeImpl(client.Pass(),
                 devtools_event_listeners,
                 port_reservation.Pass()),
      process_(process),
      command_(command) {
  if (user_data_dir->IsValid())
    CHECK(user_data_dir_.Set(user_data_dir->Take()));
  if (extension_dir->IsValid())
    CHECK(extension_dir_.Set(extension_dir->Take()));
}

ChromeDesktopImpl::~ChromeDesktopImpl() {
  if (!quit_) {
    base::FilePath user_data_dir = user_data_dir_.Take();
    base::FilePath extension_dir = extension_dir_.Take();
    LOG(WARNING) << "chrome detaches, user should take care of directory:"
                 << user_data_dir.value() << " and " << extension_dir.value();
  }
  base::CloseProcessHandle(process_);
}

Status ChromeDesktopImpl::WaitForPageToLoad(const std::string& url,
                                            const base::TimeDelta& timeout,
                                            scoped_ptr<WebView>* web_view) {
  base::TimeTicks deadline = base::TimeTicks::Now() + timeout;
  std::string id;
  while (base::TimeTicks::Now() < deadline) {
    WebViewsInfo views_info;
    Status status = devtools_http_client_->GetWebViewsInfo(&views_info);
    if (status.IsError())
      return status;

    for (size_t i = 0; i < views_info.GetSize(); ++i) {
      if (views_info.Get(i).url.find(url) == 0) {
        id = views_info.Get(i).id;
        break;
      }
    }
    if (!id.empty())
      break;
    base::PlatformThread::Sleep(base::TimeDelta::FromMilliseconds(100));
  }
  if (id.empty())
    return Status(kUnknownError, "page could not be found: " + url);

  scoped_ptr<WebView> web_view_tmp(new WebViewImpl(
      id, GetBuildNo(), devtools_http_client_->CreateClient(id)));
  Status status = web_view_tmp->ConnectIfNecessary();
  if (status.IsError())
    return status;

  status = web_view_tmp->WaitForPendingNavigations(
      std::string(), deadline - base::TimeTicks::Now(), false);
  if (status.IsOk())
    *web_view = web_view_tmp.Pass();
  return status;
}

Status ChromeDesktopImpl::GetAutomationExtension(
    AutomationExtension** extension) {
  if (!automation_extension_) {
    scoped_ptr<WebView> web_view;
    Status status = WaitForPageToLoad(
        "chrome-extension://aapnijgdinlhnhlmodcfapnahmbfebeb/"
        "_generated_background_page.html",
        base::TimeDelta::FromSeconds(10),
        &web_view);
    if (status.IsError())
      return Status(kUnknownError, "cannot get automation extension", status);

    automation_extension_.reset(new AutomationExtension(web_view.Pass()));
  }
  *extension = automation_extension_.get();
  return Status(kOk);
}

ChromeDesktopImpl* ChromeDesktopImpl::GetAsDesktop() {
  return this;
}

std::string ChromeDesktopImpl::GetOperatingSystemName() {
  return base::SysInfo::OperatingSystemName();
}

Status ChromeDesktopImpl::QuitImpl() {
  if (!KillProcess(process_))
    return Status(kUnknownError, "cannot kill Chrome");
  return Status(kOk);
}

const CommandLine& ChromeDesktopImpl::command() const {
  return command_;
}
