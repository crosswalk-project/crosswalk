// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service_provider.h"

#if defined(OS_LINUX)
#include "xwalk/application/browser/application_service_provider_linux.h"
#endif

namespace xwalk {
namespace application {

ApplicationServiceProvider::ApplicationServiceProvider(
    ApplicationService* app_service)
    : app_service_(app_service) {}

ApplicationServiceProvider::~ApplicationServiceProvider() {}

scoped_ptr<ApplicationServiceProvider> ApplicationServiceProvider::Create(
    ApplicationService* app_service) {
#if defined(OS_LINUX)
  return scoped_ptr<ApplicationServiceProvider>(
      new ApplicationServiceProviderLinux(app_service));
#else
  return scoped_ptr<ApplicationServiceProvider>();
#endif
}

}  // namespace application
}  // namespace xwalk
