// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service_provider.h"

namespace xwalk {
namespace application {

ApplicationServiceProvider::ApplicationServiceProvider(
    ApplicationService* app_service)
    : app_service_(app_service) {}

ApplicationServiceProvider::~ApplicationServiceProvider() {}

scoped_ptr<ApplicationServiceProvider> ApplicationServiceProvider::Create(
    ApplicationService* app_service) {
  return scoped_ptr<ApplicationServiceProvider>();
}

}  // namespace application
}  // namespace xwalk
