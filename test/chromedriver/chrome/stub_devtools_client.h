// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_CHROME_STUB_DEVTOOLS_CLIENT_H_
#define CHROME_TEST_CHROMEDRIVER_CHROME_STUB_DEVTOOLS_CLIENT_H_

#include <list>
#include <string>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/test/chromedriver/chrome/devtools_client.h"

namespace base {
class DictionaryValue;
}

class Status;

class StubDevToolsClient : public DevToolsClient {
 public:
  explicit StubDevToolsClient(const std::string& id);
  StubDevToolsClient();
  virtual ~StubDevToolsClient();

  // Overridden from DevToolsClient:
  virtual const std::string& GetId() OVERRIDE;
  virtual bool WasCrashed() OVERRIDE;
  virtual Status ConnectIfNecessary() OVERRIDE;
  virtual Status SendCommand(const std::string& method,
                             const base::DictionaryValue& params) OVERRIDE;
  virtual Status SendCommandAndGetResult(
      const std::string& method,
      const base::DictionaryValue& params,
      scoped_ptr<base::DictionaryValue>* result) OVERRIDE;
  virtual void AddListener(DevToolsEventListener* listener) OVERRIDE;
  virtual Status HandleEventsUntil(const ConditionalFunc& conditional_func,
                                   const base::TimeDelta& timeout) OVERRIDE;
  virtual Status HandleReceivedEvents() OVERRIDE;

 protected:
  const std::string id_;
  std::list<DevToolsEventListener*> listeners_;
};

#endif  // CHROME_TEST_CHROMEDRIVER_CHROME_STUB_DEVTOOLS_CLIENT_H_
