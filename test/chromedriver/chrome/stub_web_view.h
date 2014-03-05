// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_STUB_WEB_VIEW_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_STUB_WEB_VIEW_H_

#include <list>
#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/test/chromedriver/chrome/web_view.h"

class StubWebView : public WebView {
 public:
  explicit StubWebView(const std::string& id);
  virtual ~StubWebView();

  // Overridden from WebView:
  virtual std::string GetId() OVERRIDE;
  virtual bool WasCrashed() OVERRIDE;
  virtual Status ConnectIfNecessary() OVERRIDE;
  virtual Status HandleReceivedEvents() OVERRIDE;
  virtual Status Load(const std::string& url) OVERRIDE;
  virtual Status Reload() OVERRIDE;
  virtual Status EvaluateScript(const std::string& frame,
                                const std::string& function,
                                scoped_ptr<base::Value>* result) OVERRIDE;
  virtual Status CallFunction(const std::string& frame,
                              const std::string& function,
                              const base::ListValue& args,
                              scoped_ptr<base::Value>* result) OVERRIDE;
  virtual Status CallAsyncFunction(const std::string& frame,
                                   const std::string& function,
                                   const base::ListValue& args,
                                   const base::TimeDelta& timeout,
                                   scoped_ptr<base::Value>* result) OVERRIDE;
  virtual Status CallUserAsyncFunction(
      const std::string& frame,
      const std::string& function,
      const base::ListValue& args,
      const base::TimeDelta& timeout,
      scoped_ptr<base::Value>* result) OVERRIDE;
  virtual Status GetFrameByFunction(const std::string& frame,
                                    const std::string& function,
                                    const base::ListValue& args,
                                    std::string* out_frame) OVERRIDE;
  virtual Status DispatchMouseEvents(
      const std::list<MouseEvent>& events, const std::string& frame) OVERRIDE;
  virtual Status DispatchTouchEvents(
      const std::list<TouchEvent>& events) OVERRIDE;
  virtual Status DispatchKeyEvents(const std::list<KeyEvent>& events) OVERRIDE;
  virtual Status GetCookies(scoped_ptr<base::ListValue>* cookies) OVERRIDE;
  virtual Status DeleteCookie(const std::string& name,
                              const std::string& url) OVERRIDE;
  virtual Status WaitForPendingNavigations(const std::string& frame_id,
                                           const base::TimeDelta& timeout,
                                           bool stop_load_on_timeout) OVERRIDE;
  virtual Status IsPendingNavigation(
      const std::string& frame_id, bool* is_pending) OVERRIDE;
  virtual JavaScriptDialogManager* GetJavaScriptDialogManager() OVERRIDE;
  virtual Status OverrideGeolocation(const Geoposition& geoposition) OVERRIDE;
  virtual Status CaptureScreenshot(std::string* screenshot) OVERRIDE;
  virtual Status SetFileInputFiles(
      const std::string& frame,
      const base::DictionaryValue& element,
      const std::vector<base::FilePath>& files) OVERRIDE;
  virtual Status TakeHeapSnapshot(scoped_ptr<base::Value>* snapshot) OVERRIDE;

 private:
  std::string id_;
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_STUB_WEB_VIEW_H_
