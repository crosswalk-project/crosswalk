// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/stub_xwalk.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/xwalk/web_view.h"

StubXwalk::StubXwalk() {}

StubXwalk::~StubXwalk() {}

XwalkDesktopImpl* StubXwalk::GetAsDesktop() {
  return NULL;
}

std::string StubXwalk::GetVersion() {
  return std::string();
}

int StubXwalk::GetBuildNo() {
  return 9999;
}

bool StubXwalk::HasCrashedWebView() {
  return false;
}

Status StubXwalk::GetWebViewIds(std::list<std::string>* web_view_ids) {
  return Status(kOk);
}

Status StubXwalk::GetWebViewById(const std::string& id, WebView** web_view) {
  return Status(kOk);
}

Status StubXwalk::CloseWebView(const std::string& id) {
  return Status(kOk);
}

Status StubXwalk::ActivateWebView(const std::string& id) {
  return Status(kOk);
}

std::string StubXwalk::GetOperatingSystemName() {
  return std::string();
}

Status StubXwalk::Quit() {
  return Status(kOk);
}
