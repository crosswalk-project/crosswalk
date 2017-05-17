// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/test/xwalkdriver/xwalk/navigation_tracker.h"

#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "xwalk/test/xwalkdriver/xwalk/devtools_client.h"
#include "xwalk/test/xwalkdriver/xwalk/status.h"

NavigationTracker::NavigationTracker(DevToolsClient* client)
    : client_(client),
      loading_state_(kUnknown) {
  client_->AddListener(this);
}

NavigationTracker::NavigationTracker(DevToolsClient* client,
                                     LoadingState known_state)
    : client_(client),
      loading_state_(known_state) {
  client_->AddListener(this);
}

NavigationTracker::~NavigationTracker() {}

Status NavigationTracker::IsPendingNavigation(const std::string& frame_id,
                                              bool* is_pending) {
  if (loading_state_ == kUnknown) {
    // If the loading state is unknown (which happens after first connecting),
    // force loading to start and set the state to loading. This will
    // cause a frame start event to be received, and the frame stop event
    // will not be received until all frames are loaded.
    // Loading is forced to start by attaching a temporary iframe.
    // Forcing loading to start is not necessary if the main frame is not yet
    // loaded.
    const char kStartLoadingIfMainFrameNotLoading[] =
       "var isLoaded = document.readyState == 'complete' ||"
       "    document.readyState == 'interactive';"
       "if (isLoaded) {"
       "  var frame = document.createElement('iframe');"
       "  frame.src = 'about:blank';"
       "  document.body.appendChild(frame);"
       "  window.setTimeout(function() {"
       "    document.body.removeChild(frame);"
       "  }, 0);"
       "}";
    base::DictionaryValue params;
    params.SetString("expression", kStartLoadingIfMainFrameNotLoading);
    scoped_ptr<base::DictionaryValue> result;
    Status status = client_->SendCommandAndGetResult(
        "Runtime.evaluate", params, &result);
    if (status.IsError())
      return Status(kUnknownError, "cannot determine loading status", status);

    // Between the time the JavaScript is evaluated and SendCommandAndGetResult
    // returns, OnEvent may have received info about the loading state.
    // This is only possible during a nested command. Only set the loading state
    // if the loading state is still unknown.
    if (loading_state_ == kUnknown)
      loading_state_ = kLoading;
  }
  *is_pending = loading_state_ == kLoading;
  if (frame_id.empty())
    *is_pending |= scheduled_frame_set_.size() > 0;
  else
    *is_pending |= scheduled_frame_set_.count(frame_id) > 0;
  return Status(kOk);
}

Status NavigationTracker::OnConnected(DevToolsClient* client) {
  loading_state_ = kUnknown;
  scheduled_frame_set_.clear();

  // Enable page domain notifications to allow tracking navigation state.
  base::DictionaryValue empty_params;
  return client_->SendCommand("Page.enable", empty_params);
}

Status NavigationTracker::OnEvent(DevToolsClient* client,
                                  const std::string& method,
                                  const base::DictionaryValue& params) {
  // Xwalk does not send Page.frameStoppedLoading until all frames have
  // run their onLoad handlers (including frames created during the handlers).
  // When it does, it only sends one stopped event for all frames.
  if (method == "Page.frameStartedLoading") {
    loading_state_ = kLoading;
  } else if (method == "Page.frameStoppedLoading") {
    loading_state_ = kNotLoading;
  } else if (method == "Page.frameScheduledNavigation") {
    double delay;
    if (!params.GetDouble("delay", &delay))
      return Status(kUnknownError, "missing or invalid 'delay'");

    std::string frame_id;
    if (!params.GetString("frameId", &frame_id))
      return Status(kUnknownError, "missing or invalid 'frameId'");

    // WebDriver spec says to ignore redirects over 1s.
    if (delay > 1)
      return Status(kOk);
    scheduled_frame_set_.insert(frame_id);
  } else if (method == "Page.frameClearedScheduledNavigation") {
    std::string frame_id;
    if (!params.GetString("frameId", &frame_id))
      return Status(kUnknownError, "missing or invalid 'frameId'");

    scheduled_frame_set_.erase(frame_id);
  } else if (method == "Page.frameNavigated") {
    // Note: in some cases Page.frameNavigated may be received for subframes
    // without a frameStoppedLoading (for example cnn.com).

    // If the main frame just navigated, discard any pending scheduled
    // navigations. For some reasons at times the cleared event is not
    // received when navigating.
    // See crbug.com/180742.
    const base::Value* unused_value;
    if (!params.Get("frame.parentId", &unused_value))
      scheduled_frame_set_.clear();
  } else if (method == "Inspector.targetCrashed") {
    loading_state_ = kNotLoading;
    scheduled_frame_set_.clear();
  }
  return Status(kOk);
}

Status NavigationTracker::OnCommandSuccess(DevToolsClient* client,
                                           const std::string& method) {
  if (method == "Page.navigate" && loading_state_ != kLoading) {
    // At this point the browser has initiated the navigation, but besides that,
    // it is unknown what will happen.
    //
    // There are a few cases (perhaps more):
    // 1 The RenderViewHost has already queued ViewMsg_Navigate and loading
    //   will start shortly.
    // 2 The RenderViewHost has already queued ViewMsg_Navigate and loading
    //   will never start because it is just an in-page fragment navigation.
    // 3 The RenderViewHost is suspended and hasn't queued ViewMsg_Navigate
    //   yet. This happens for cross-site navigations. The RenderViewHost
    //   will not queue ViewMsg_Navigate until it is ready to unload the
    //   previous page (after running unload handlers and such).
    //
    // To determine whether a load is expected, do a round trip to the
    // renderer to ask what the URL is.
    // If case #1, by the time the command returns, the frame started to load
    // event will also have been received, since the DevTools command will
    // be queued behind ViewMsg_Navigate.
    // If case #2, by the time the command returns, the navigation will
    // have already happened, although no frame start/stop events will have
    // been received.
    // If case #3, the URL will be blank if the navigation hasn't been started
    // yet. In that case, expect a load to happen in the future.
    loading_state_ = kUnknown;
    base::DictionaryValue params;
    params.SetString("expression", "document.URL");
    scoped_ptr<base::DictionaryValue> result;
    Status status = client_->SendCommandAndGetResult(
        "Runtime.evaluate", params, &result);
    std::string url;
    if (status.IsError() || !result->GetString("result.value", &url))
      return Status(kUnknownError, "cannot determine loading status", status);
    if (loading_state_ == kUnknown && url.empty())
      loading_state_ = kLoading;
  }
  return Status(kOk);
}
