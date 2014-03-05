// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_IMPL_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_IMPL_H_

#include <list>
#include <string>

#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "xwalk/test/chromedriver/chrome/chrome.h"

class AutomationExtension;
class DevToolsEventListener;
class DevToolsHttpClient;
class JavaScriptDialogManager;
class PortReservation;
class Status;
class WebView;
class WebViewImpl;

class ChromeImpl : public Chrome {
 public:
  virtual ~ChromeImpl();

  // Overridden from Chrome:
  virtual std::string GetVersion() OVERRIDE;
  virtual ChromeDesktopImpl* GetAsDesktop() OVERRIDE;
  virtual int GetBuildNo() OVERRIDE;
  virtual bool HasCrashedWebView() OVERRIDE;
  virtual Status GetWebViewIds(std::list<std::string>* web_view_ids) OVERRIDE;
  virtual Status GetWebViewById(const std::string& id,
                                WebView** web_view) OVERRIDE;
  virtual Status CloseWebView(const std::string& id) OVERRIDE;
  virtual Status ActivateWebView(const std::string& id) OVERRIDE;
  virtual Status Quit() OVERRIDE;

 protected:
  ChromeImpl(
      scoped_ptr<DevToolsHttpClient> client,
      ScopedVector<DevToolsEventListener>& devtools_event_listeners,
      scoped_ptr<PortReservation> port_reservation);

  virtual Status QuitImpl() = 0;

  bool quit_;
  scoped_ptr<DevToolsHttpClient> devtools_http_client_;

 private:
  typedef std::list<linked_ptr<WebViewImpl> > WebViewList;

  // Web views in this list are in the same order as they are opened.
  WebViewList web_views_;
  ScopedVector<DevToolsEventListener> devtools_event_listeners_;
  scoped_ptr<PortReservation> port_reservation_;
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_CHROME_IMPL_H_
