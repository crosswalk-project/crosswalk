// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_STUB_XWALK_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_STUB_XWALK_H_

#include <list>
#include <string>

#include "base/compiler_specific.h"
#include "xwalk/test/xwalkdriver/xwalk/xwalk.h"

class Status;
class WebView;

class StubXwalk : public Xwalk {
 public:
  StubXwalk();
  virtual ~StubXwalk();

  // Overridden from Xwalk:
  virtual XwalkDesktopImpl* GetAsDesktop() OVERRIDE;
  virtual std::string GetVersion() OVERRIDE;
  virtual int GetBuildNo() OVERRIDE;
  virtual bool HasCrashedWebView() OVERRIDE;
  virtual Status GetWebViewIds(std::list<std::string>* web_view_ids) OVERRIDE;
  virtual Status GetWebViewById(const std::string& id,
                                WebView** web_view) OVERRIDE;
  virtual Status CloseWebView(const std::string& id) OVERRIDE;
  virtual Status ActivateWebView(const std::string& id) OVERRIDE;
  virtual std::string GetOperatingSystemName() OVERRIDE;
  virtual Status Quit() OVERRIDE;
};

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_STUB_XWALK_H_
