// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_XWALK_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_XWALK_H_

#include <list>
#include <string>

class XwalkDesktopImpl;
class Status;
class WebView;

class Xwalk {
 public:
  virtual ~Xwalk() {}

  virtual XwalkDesktopImpl* GetAsDesktop() = 0;

  virtual std::string GetVersion() = 0;

  virtual int GetBuildNo() = 0;

  virtual bool HasCrashedWebView() = 0;

  // Return ids of opened WebViews in the same order as they are opened.
  virtual Status GetWebViewIds(std::list<std::string>* web_view_ids) = 0;

  // Return the WebView for the given id.
  virtual Status GetWebViewById(const std::string& id, WebView** web_view) = 0;

  // Closes the specified WebView.
  virtual Status CloseWebView(const std::string& id) = 0;

  // Activates the specified WebView.
  virtual Status ActivateWebView(const std::string& id) = 0;

  // Get the operation system where Xwalk is running.
  virtual std::string GetOperatingSystemName() = 0;

  // Quits Xwalk.
  virtual Status Quit() = 0;
};

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_XWALK_H_
