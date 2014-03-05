// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_EXISTING_IMPL_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_EXISTING_IMPL_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/chromedriver/chrome/chrome_impl.h"

class DevToolsHttpClient;

class XwalkExistingImpl : public ChromeImpl {
 public:
  XwalkExistingImpl(
      scoped_ptr<DevToolsHttpClient> client,
      ScopedVector<DevToolsEventListener>& devtools_event_listeners);
  virtual ~XwalkExistingImpl();

  // Overridden from Xwalk.
  virtual std::string GetOperatingSystemName() OVERRIDE;

  // Overridden from XwalkImpl.
  virtual Status QuitImpl() OVERRIDE;
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_EXISTING_IMPL_H_
