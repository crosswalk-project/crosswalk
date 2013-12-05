// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/geolocation/tizen/location_provider_tizen.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/time/time.h"
#include "content/public/common/geoposition.h"

namespace xwalk {

LocationProviderTizen::LocationProviderTizen()
  : manager_(NULL),
    geolocation_message_loop_(base::MessageLoop::current()),
    is_permission_granted_(false) {}

LocationProviderTizen::~LocationProviderTizen() {
  StopProvider();
}

bool LocationProviderTizen::StartProvider(bool high_accuracy) {
  // Tizen location framework doesn't have API that could be used to set
  // preferred accuracy. Accuracy would be provided by the framework when
  // position is updated.
  if (InitLocationManager() && is_permission_granted_)
    return location_manager_start(manager_) == LOCATIONS_ERROR_NONE;

  return false;
}

void LocationProviderTizen::StopProvider() {
  if (manager_) {
    location_manager_unset_position_updated_cb(manager_);
    location_manager_stop(manager_);
    location_manager_destroy(manager_);
    manager_ = NULL;
  } else {
    LOG(WARNING) << "Location manager not initialized.";
  }
}

void LocationProviderTizen::GetPosition(content::Geoposition* position) {
  *position = last_position_;
}

void LocationProviderTizen::RequestRefresh() {
  // Tizen location framework sends updates automatically. No need to refresh.
}

void LocationProviderTizen::OnPermissionGranted() {
  is_permission_granted_ = true;
  if (manager_)
    StartProvider(true);
}

bool LocationProviderTizen::InitLocationManager() {
  if (manager_)
    return true;

  // FIXME(shalamov): Tizen location manager throws critical error when
  // hybrid location method is used. Using GPS until it is fixed.
  int ret = location_manager_create(LOCATIONS_METHOD_GPS, &manager_);
  if (ret != LOCATIONS_ERROR_NONE) {
    LOG(ERROR) << "Cannot create location manager.";
    manager_ = NULL;
    return false;
  }

  ret = location_manager_set_service_state_changed_cb(manager_,
         &LocationProviderTizen::OnStateChanged,
         this);

  if (ret != LOCATIONS_ERROR_NONE) {
    location_manager_unset_position_updated_cb(manager_);
    location_manager_destroy(manager_);
    manager_ = NULL;
    return false;
  }

  return true;
}

void LocationProviderTizen::NotifyLocationProvider() {
  DCHECK(manager_);
  int ret;
  content::Geoposition pos;
  time_t timestamp;
  ret = location_manager_get_position(manager_,
                                      &pos.altitude,
                                      &pos.latitude,
                                      &pos.longitude,
                                      &timestamp);
  pos.timestamp = base::Time::FromTimeT(timestamp);

  if (ret != LOCATIONS_ERROR_NONE) {
    LOG(ERROR) << "Cannot retrieve position from location manager.";
    return;
  }

  location_accuracy_level_e level;
  double horizontal;
  double vertical;
  ret = location_manager_get_accuracy(manager_,
                                      &level,
                                      &horizontal,
                                      &vertical);

  if (ret != LOCATIONS_ERROR_NONE)
    LOG(ERROR) << "Cannot retrieve position accuracy from location manager.";
  else
    pos.accuracy = (horizontal / 2) + (vertical / 2);

  last_position_ = pos;
  if (geolocation_message_loop_) {
    base::Closure task = base::Bind(&LocationProviderTizen::NotifyCallback,
                                    base::Unretained(this),
                                    last_position_);
    geolocation_message_loop_->PostTask(FROM_HERE, task);
  }
}

void LocationProviderTizen::OnStateChanged(location_service_state_e state,
                                           void* data) {
  DCHECK(data);
  LocationProviderTizen* impl = static_cast<LocationProviderTizen*>(data);
  if (state == LOCATIONS_SERVICE_ENABLED)
    impl->NotifyLocationProvider();
}

}  // namespace xwalk

namespace content {
__attribute__((visibility("default"))) content::LocationProvider*
    NewSystemLocationProvider() {
  return new xwalk::LocationProviderTizen;
}

}  // namespace content
