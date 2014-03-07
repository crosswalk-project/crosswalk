// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>
#include <string>

#include "base/memory/scoped_ptr.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"
#include "xwalk/test/xwalkdriver/xwalk/stub_xwalk.h"
#include "xwalk/test/xwalkdriver/xwalk/stub_web_view.h"
#include "xwalk/test/xwalkdriver/session.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class MockXwalk : public StubXwalk {
 public:
  MockXwalk() : web_view_("1") {}
  virtual ~MockXwalk() {}

  virtual Status GetWebViewById(const std::string& id,
                                WebView** web_view) OVERRIDE {
    if (id == web_view_.GetId()) {
      *web_view = &web_view_;
      return Status(kOk);
    }
    return Status(kUnknownError);
  }

 private:
  StubWebView web_view_;
};

}  // namespace

TEST(Session, GetTargetWindowNoXwalk) {
  Session session("1");
  WebView* web_view;
  ASSERT_EQ(kNoSuchWindow, session.GetTargetWindow(&web_view).code());
}

TEST(Session, GetTargetWindowTargetWindowClosed) {
  scoped_ptr<Xwalk> xwalk(new MockXwalk());
  Session session("1", xwalk.Pass());
  session.window = "2";
  WebView* web_view;
  ASSERT_EQ(kNoSuchWindow, session.GetTargetWindow(&web_view).code());
}

TEST(Session, GetTargetWindowTargetWindowStillOpen) {
  scoped_ptr<Xwalk> xwalk(new MockXwalk());
  Session session("1", xwalk.Pass());
  session.window = "1";
  WebView* web_view = NULL;
  ASSERT_EQ(kOk, session.GetTargetWindow(&web_view).code());
  ASSERT_TRUE(web_view);
}
