// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_DESKTOP_IMPL_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_DESKTOP_IMPL_H_

#include <string>

#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/process/process.h"
#include "xwalk/test/chromedriver/chrome/chrome_impl.h"

namespace base {
class TimeDelta;
}

class AutomationExtension;
class DevToolsHttpClient;
class Status;
class WebView;

class ChromeDesktopImpl : public ChromeImpl {
 public:
  ChromeDesktopImpl(
      scoped_ptr<DevToolsHttpClient> client,
      ScopedVector<DevToolsEventListener>& devtools_event_listeners,
      scoped_ptr<PortReservation> port_reservation,
      base::ProcessHandle process,
      const CommandLine& command,
      base::ScopedTempDir* user_data_dir,
      base::ScopedTempDir* extension_dir);
  virtual ~ChromeDesktopImpl();

  // Waits for a page with the given URL to appear and finish loading.
  // Returns an error if the timeout is exceeded.
  Status WaitForPageToLoad(const std::string& url,
                           const base::TimeDelta& timeout,
                           scoped_ptr<WebView>* web_view);

  // Gets the installed automation extension.
  Status GetAutomationExtension(AutomationExtension** extension);

  // Overridden from Chrome:
  virtual ChromeDesktopImpl* GetAsDesktop() OVERRIDE;
  virtual std::string GetOperatingSystemName() OVERRIDE;

  // Overridden from ChromeImpl:
  virtual Status QuitImpl() OVERRIDE;

  const CommandLine& command() const;

 private:
  base::ProcessHandle process_;
  CommandLine command_;
  base::ScopedTempDir user_data_dir_;
  base::ScopedTempDir extension_dir_;

  // Lazily initialized, may be null.
  scoped_ptr<AutomationExtension> automation_extension_;
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_DESKTOP_IMPL_H_
