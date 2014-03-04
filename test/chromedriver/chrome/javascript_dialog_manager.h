// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_JAVASCRIPT_DIALOG_MANAGER_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_JAVASCRIPT_DIALOG_MANAGER_H_

#include <list>
#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "chrome/test/chromedriver/chrome/devtools_event_listener.h"

namespace base {
class DictionaryValue;
}

class DevToolsClient;
class Status;

// Tracks the opening and closing of JavaScript dialogs (e.g., alerts).
class JavaScriptDialogManager : public DevToolsEventListener {
 public:
  explicit JavaScriptDialogManager(DevToolsClient* client);
  virtual ~JavaScriptDialogManager();

  bool IsDialogOpen();

  Status GetDialogMessage(std::string* message);

  Status HandleDialog(bool accept, const std::string* text);

  // Overridden from DevToolsEventListener:
  virtual Status OnConnected(DevToolsClient* client) OVERRIDE;
  virtual Status OnEvent(DevToolsClient* client,
                         const std::string& method,
                         const base::DictionaryValue& params) OVERRIDE;

 private:
  DevToolsClient* client_;

  // The queue of unhandled dialogs. This may be greater than 1 in rare
  // cases. E.g., if the page shows an alert but before the manager received
  // the event, a script was injected via Inspector that triggered an alert.
  std::list<std::string> unhandled_dialog_queue_;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialogManager);
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_JAVASCRIPT_DIALOG_MANAGER_H_
