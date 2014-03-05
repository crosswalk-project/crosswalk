// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/chromedriver/session.h"

#include <list>

#include "base/lazy_instance.h"
#include "base/threading/thread_local.h"
#include "base/values.h"
#include "xwalk/test/chromedriver/chrome/chrome.h"
#include "xwalk/test/chromedriver/chrome/status.h"
#include "xwalk/test/chromedriver/chrome/version.h"
#include "xwalk/test/chromedriver/chrome/web_view.h"
#include "xwalk/test/chromedriver/logging.h"

namespace {

base::LazyInstance<base::ThreadLocalPointer<Session> >
    lazy_tls_session = LAZY_INSTANCE_INITIALIZER;

}  // namespace

FrameInfo::FrameInfo(const std::string& parent_frame_id,
                     const std::string& frame_id,
                     const std::string& chromedriver_frame_id)
    : parent_frame_id(parent_frame_id),
      frame_id(frame_id),
      chromedriver_frame_id(chromedriver_frame_id) {}

const base::TimeDelta Session::kDefaultPageLoadTimeout =
    base::TimeDelta::FromMinutes(5);

Session::Session(const std::string& id)
    : id(id),
      quit(false),
      detach(false),
      force_devtools_screenshot(false),
      sticky_modifiers(0),
      mouse_position(0, 0),
      page_load_timeout(kDefaultPageLoadTimeout) {}

Session::Session(const std::string& id, scoped_ptr<Chrome> chrome)
    : id(id),
      quit(false),
      detach(false),
      force_devtools_screenshot(false),
      chrome(chrome.Pass()),
      sticky_modifiers(0),
      mouse_position(0, 0),
      page_load_timeout(kDefaultPageLoadTimeout) {}

Session::~Session() {}

Status Session::GetTargetWindow(WebView** web_view) {
  if (!chrome)
    return Status(kNoSuchWindow, "no chrome started in this session");

  Status status = chrome->GetWebViewById(window, web_view);
  if (status.IsError())
    status = Status(kNoSuchWindow, "target window already closed", status);
  return status;
}

void Session::SwitchToTopFrame() {
  frames.clear();
}

void Session::SwitchToSubFrame(const std::string& frame_id,
                               const std::string& chromedriver_frame_id) {
  std::string parent_frame_id;
  if (!frames.empty())
    parent_frame_id = frames.back().frame_id;
  frames.push_back(FrameInfo(parent_frame_id, frame_id, chromedriver_frame_id));
}

std::string Session::GetCurrentFrameId() const {
  if (frames.empty())
    return std::string();
  return frames.back().frame_id;
}

std::vector<WebDriverLog*> Session::GetAllLogs() const {
  std::vector<WebDriverLog*> logs;
  for (ScopedVector<WebDriverLog>::const_iterator log = devtools_logs.begin();
       log != devtools_logs.end();
       ++log) {
    logs.push_back(*log);
  }
  if (driver_log)
    logs.push_back(driver_log.get());
  return logs;
}

Session* GetThreadLocalSession() {
  return lazy_tls_session.Pointer()->Get();
}

void SetThreadLocalSession(scoped_ptr<Session> session) {
  lazy_tls_session.Pointer()->Set(session.release());
}
