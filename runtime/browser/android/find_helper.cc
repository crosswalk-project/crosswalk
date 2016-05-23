// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is a copy of android_webview/browser/find_helper.cc

#include "xwalk/runtime/browser/android/find_helper.h"

#include "base/message_loop/message_loop.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/stop_find_action.h"
#include "third_party/WebKit/public/web/WebFindOptions.h"

using content::WebContents;
using blink::WebFindOptions;

namespace xwalk {

FindHelper::FindHelper(WebContents* web_contents)
    : WebContentsObserver(web_contents),
      listener_(nullptr),
      async_find_started_(false),
      find_request_id_counter_(0),
      current_request_id_(0),
      last_match_count_(-1),
      last_active_ordinal_(-1) {
}

FindHelper::~FindHelper() {
}

void FindHelper::SetListener(Listener* listener) {
  listener_ = listener;
}

void FindHelper::FindAllAsync(const base::string16& search_string) {
  // Stop any ongoing asynchronous request.
  web_contents()->StopFinding(content::STOP_FIND_ACTION_KEEP_SELECTION);

  async_find_started_ = true;

  StartNewRequest(search_string);

  if (MaybeHandleEmptySearch(search_string))
    return;

  WebFindOptions options;
  options.forward = true;
  options.matchCase = false;
  options.findNext = false;

  web_contents()->Find(current_request_id_, search_string, options);
}

void FindHelper::HandleFindReply(int request_id,
                                 int match_count,
                                 int active_ordinal,
                                 bool finished) {
  if (!async_find_started_ || request_id != current_request_id_)
    return;

  NotifyResults(active_ordinal, match_count, finished);
}

void FindHelper::FindNext(bool forward) {
  if (!async_find_started_)
    return;

  if (MaybeHandleEmptySearch(last_search_string_))
    return;

  WebFindOptions options;
  options.forward = forward;
  options.matchCase = false;
  options.findNext = true;

  web_contents()->Find(current_request_id_, last_search_string_, options);
}

void FindHelper::ClearMatches() {
  web_contents()->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);

  async_find_started_ = false;
  last_search_string_.clear();
  last_match_count_ = -1;
  last_active_ordinal_ = -1;
}

bool FindHelper::MaybeHandleEmptySearch(const base::string16& search_string) {
  if (!search_string.empty())
    return false;

  web_contents()->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);
  NotifyResults(0, 0, true);
  return true;
}

void FindHelper::StartNewRequest(const base::string16& search_string) {
  current_request_id_ = find_request_id_counter_++;
  last_search_string_ = search_string;
  last_match_count_ = -1;
  last_active_ordinal_ = -1;
}

void FindHelper::NotifyResults(int active_ordinal,
                               int match_count,
                               bool finished) {
  // Match count or ordinal values set to -1 refer to received replies.
  if (match_count == -1)
    match_count = last_match_count_;
  else
    last_match_count_ = match_count;

  if (active_ordinal == -1)
    active_ordinal = last_active_ordinal_;
  else
    last_active_ordinal_ = active_ordinal;

  // Skip the update if we don't still have a valid ordinal.
  // The next update, final or not, should have this information.
  if (!finished && active_ordinal == -1)
    return;

  // Safeguard in case of errors to prevent reporting -1 to the API listeners.
  if (match_count == -1) {
    NOTREACHED();
    match_count = 0;
  }

  if (active_ordinal == -1) {
    NOTREACHED();
    active_ordinal = 0;
  }

  // WebView.FindListener active match ordinals are 0-based while WebKit sends
  // 1-based ordinals. Still we can receive 0 ordinal in case of no results.
  active_ordinal = std::max(active_ordinal - 1, 0);

  if (listener_)
    listener_->OnFindResultReceived(active_ordinal, match_count, finished);
}

}  // namespace xwalk
