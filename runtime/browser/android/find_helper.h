// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is a copy of android_webview/browser/find_helper.h

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_FIND_HELPER_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_FIND_HELPER_H_

#include "base/macros.h"
#include "content/public/browser/web_contents_observer.h"

namespace xwalk {

// Handles the WebView find-in-page API requests.
class FindHelper : public content::WebContentsObserver {
 public:
  class Listener {
   public:
    // Called when receiving a new find-in-page update.
    // This will be triggered when the results of FindAllSync, FindAllAsync and
    // FindNext are available. The value provided in active_ordinal is 0-based.
    virtual void OnFindResultReceived(int active_ordinal,
                                      int match_count,
                                      bool finished) = 0;
    virtual ~Listener() {}
  };

  explicit FindHelper(content::WebContents* web_contents);
  ~FindHelper() override;

  // Sets the listener to receive find result updates.
  // Does not own the listener and must set to nullptr when invalid.
  void SetListener(Listener* listener);

  // Asynchronous API.
  void FindAllAsync(const base::string16& search_string);
  void HandleFindReply(int request_id,
                       int match_count,
                       int active_ordinal,
                       bool finished);

  // Methods valid in both synchronous and asynchronous modes.
  void FindNext(bool forward);
  void ClearMatches();

 private:
  void StartNewRequest(const base::string16& search_string);
  bool MaybeHandleEmptySearch(const base::string16& search_string);
  void NotifyResults(int active_ordinal, int match_count, bool finished);

  // Listener results are reported to.
  Listener* listener_;

  // Used to check the validity of FindNext operations.
  bool async_find_started_;

  // Used to provide different ids to each request and for result
  // verification in asynchronous calls.
  int find_request_id_counter_;
  int current_request_id_;

  // Required by FindNext and the incremental find replies.
  base::string16 last_search_string_;
  int last_match_count_;
  int last_active_ordinal_;

  DISALLOW_COPY_AND_ASSIGN(FindHelper);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_FIND_HELPER_H_
