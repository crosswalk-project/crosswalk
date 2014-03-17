// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_XWALK_DESKTOP_IMPL_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_XWALK_DESKTOP_IMPL_H_

#include <string>

#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/process/process.h"
#include "xwalk/test/xwalkdriver/xwalk/xwalk_impl.h"

namespace base {
class TimeDelta;
}

class DevToolsHttpClient;
class Status;
class WebView;

class XwalkDesktopImpl : public XwalkImpl {
 public:
  XwalkDesktopImpl(
      scoped_ptr<DevToolsHttpClient> client,
      ScopedVector<DevToolsEventListener>& devtools_event_listeners,
      scoped_ptr<PortReservation> port_reservation,
      base::ProcessHandle process,
      const CommandLine& command,
      base::ScopedTempDir* extension_dir);
  virtual ~XwalkDesktopImpl();

  // Waits for a page with the given URL to appear and finish loading.
  // Returns an error if the timeout is exceeded.
  Status WaitForPageToLoad(const std::string& url,
                           const base::TimeDelta& timeout,
                           scoped_ptr<WebView>* web_view);

  // Overridden from Xwalk
  virtual XwalkDesktopImpl* GetAsDesktop() OVERRIDE;
  virtual std::string GetOperatingSystemName() OVERRIDE;

  // Overridden from XwalkImpl:
  virtual Status QuitImpl() OVERRIDE;

  const CommandLine& command() const;

 private:
  base::ProcessHandle process_;
  CommandLine command_;
  base::ScopedTempDir extension_dir_;
};

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_XWALK_DESKTOP_IMPL_H_
