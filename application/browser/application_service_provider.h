// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_H_

#include "base/memory/scoped_ptr.h"

namespace xwalk {
namespace application {

class ApplicationService;

// Exposes the functionality of ApplicationService to the rest of the
// system. Subclasses should provide the implementation. It is encouraged that
// they rely solely on ApplicationService if possible.
class ApplicationServiceProvider {
 public:
  static scoped_ptr<ApplicationServiceProvider> Create(
      ApplicationService* app_service);

  virtual ~ApplicationServiceProvider();

 protected:
  explicit ApplicationServiceProvider(ApplicationService* app_service);

  ApplicationService* app_service() const { return app_service_; }

 private:
  ApplicationService* app_service_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_SERVICE_PROVIDER_H_
