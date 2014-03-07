// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TEST_XWALKDRIVER_XWALK_WEB_VIEW_H_
#define XWALK_TEST_XWALKDRIVER_XWALK_WEB_VIEW_H_

#include <list>
#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"

namespace base {
class DictionaryValue;
class FilePath;
class ListValue;
class TimeDelta;
class Value;
}

class DevToolsClient;
struct Geoposition;
class JavaScriptDialogManager;
struct KeyEvent;
struct MouseEvent;
struct TouchEvent;
class Status;

class WebView {
 public:
  virtual ~WebView() {}

  // Return the id for this WebView.
  virtual std::string GetId() = 0;

  // Return true if the web view was crashed.
  virtual bool WasCrashed() = 0;

  // Make DevToolsCient connect to DevTools if it is disconnected.
  virtual Status ConnectIfNecessary() = 0;

  // Handles events that have been received but not yet handled.
  virtual Status HandleReceivedEvents() = 0;

  // Load a given URL in the main frame.
  virtual Status Load(const std::string& url) = 0;

  // Reload the current page.
  virtual Status Reload() = 0;

  // Evaluates a JavaScript expression in a specified frame and returns
  // the result. |frame| is a frame ID or an empty string for the main frame.
  // If the expression evaluates to a element, it will be bound to a unique ID
  // (per frame) and the ID will be returned.
  // |result| will never be NULL on success.
  virtual Status EvaluateScript(const std::string& frame,
                                const std::string& expression,
                                scoped_ptr<base::Value>* result) = 0;

  // Calls a JavaScript function in a specified frame with the given args and
  // returns the result. |frame| is a frame ID or an empty string for the main
  // frame. |args| may contain IDs that refer to previously returned elements.
  // These will be translated back to their referred objects before invoking the
  // function.
  // |result| will never be NULL on success.
  virtual Status CallFunction(const std::string& frame,
                              const std::string& function,
                              const base::ListValue& args,
                              scoped_ptr<base::Value>* result) = 0;

  // Calls a JavaScript function in a specified frame with the given args and
  // two callbacks. The first may be invoked with a value to return to the user.
  // The second may be used to report an error. This function waits until
  // one of the callbacks is invoked or the timeout occurs.
  // |result| will never be NULL on success.
  virtual Status CallAsyncFunction(const std::string& frame,
                                   const std::string& function,
                                   const base::ListValue& args,
                                   const base::TimeDelta& timeout,
                                   scoped_ptr<base::Value>* result) = 0;

  // Same as |CallAsyncFunction|, except no additional error callback is passed
  // to the function. Also, |kJavaScriptError| or |kScriptTimeout| is used
  // as the error code instead of |kUnknownError| in appropriate cases.
  // |result| will never be NULL on success.
  virtual Status CallUserAsyncFunction(const std::string& frame,
                                       const std::string& function,
                                       const base::ListValue& args,
                                       const base::TimeDelta& timeout,
                                       scoped_ptr<base::Value>* result) = 0;

  // Gets the frame ID for a frame element returned by invoking the given
  // JavaScript function. |frame| is a frame ID or an empty string for the main
  // frame.
  virtual Status GetFrameByFunction(const std::string& frame,
                                    const std::string& function,
                                    const base::ListValue& args,
                                    std::string* out_frame) = 0;

  // Dispatch a sequence of mouse events.
  virtual Status DispatchMouseEvents(const std::list<MouseEvent>& events,
                                     const std::string& frame) = 0;

  // Dispatch a sequence of touch events.
  virtual Status DispatchTouchEvents(const std::list<TouchEvent>& events) = 0;

  // Dispatch a sequence of key events.
  virtual Status DispatchKeyEvents(const std::list<KeyEvent>& events) = 0;

  // Return all the cookies visible to the current page.
  virtual Status GetCookies(scoped_ptr<base::ListValue>* cookies) = 0;

  // Delete the cookie with the given name.
  virtual Status DeleteCookie(const std::string& name,
                              const std::string& url) = 0;

  // Waits until all pending navigations have completed in the given frame.
  // If |frame_id| is "", waits for navigations on the main frame.
  // If a modal dialog appears while waiting, kUnexpectedAlertOpen will be
  // returned.
  // If timeout is exceeded, will return a timeout status.
  // If |stop_load_on_timeout| is true, will attempt to stop the page load on
  // timeout before returning the timeout status.
  virtual Status WaitForPendingNavigations(const std::string& frame_id,
                                           const base::TimeDelta& timeout,
                                           bool stop_load_on_timeout) = 0;

  // Returns whether the frame is pending navigation.
  virtual Status IsPendingNavigation(
      const std::string& frame_id, bool* is_pending) = 0;

  // Returns the JavaScriptDialogManager. Never null.
  virtual JavaScriptDialogManager* GetJavaScriptDialogManager() = 0;

  // Overrides normal geolocation with a given geoposition.
  virtual Status OverrideGeolocation(const Geoposition& geoposition) = 0;

  // Captures the visible portions of the web view as a base64-encoded PNG.
  virtual Status CaptureScreenshot(std::string* screenshot) = 0;

  // Set files in a file input element.
  // |element| is the WebElement JSON Object of the input element.
  virtual Status SetFileInputFiles(
      const std::string& frame,
      const base::DictionaryValue& element,
      const std::vector<base::FilePath>& files) = 0;

  // Take a heap snapshot which can build up a graph of Javascript objects.
  // A raw heap snapshot is in JSON format:
  //  1. A meta data element "snapshot" about how to parse data elements.
  //  2. Data elements: "nodes", "edges", "strings".
  virtual Status TakeHeapSnapshot(scoped_ptr<base::Value>* snapshot) = 0;
};

#endif  // XWALK_TEST_XWALKDRIVER_XWALK_WEB_VIEW_H_
