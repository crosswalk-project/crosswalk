// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_

#include <string>

#include "base/memory/linked_ptr.h"
#include "net/url_request/url_request_job_factory.h"
#include "xwalk/application/browser/application_service.h"

namespace xwalk {
namespace application {

// This class is a thread-safe cache of active application's data.
// This class is used by ApplicationProtocolHandler as it lives on IO thread
// and hence cannot access ApplicationService directly.
class ApplicationDataCache : public ApplicationService::Observer {
 public:
  scoped_refptr<ApplicationData> GetApplicationData(
      const std::string& application_id) const;
  ApplicationDataCache();
  ~ApplicationDataCache() override;

 private:
  void DidLaunchApplication(Application* app) override;
  void WillDestroyApplication(Application* app) override;

  ApplicationData::ApplicationDataMap cache_;
  mutable base::Lock lock_;
};

// Creates the handlers for the app:// scheme.
linked_ptr<net::URLRequestJobFactory::ProtocolHandler>
CreateApplicationProtocolHandler(ApplicationDataCache* cache);

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_PROTOCOLS_H_
