// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_GEOLOCATION_TIZEN_LOCATION_PROVIDER_TIZEN_H_
#define XWALK_RUNTIME_BROWSER_GEOLOCATION_TIZEN_LOCATION_PROVIDER_TIZEN_H_

#include <locations.h>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "content/browser/geolocation/location_provider_base.h"

namespace content {
struct Geoposition;
}

namespace base {
class MessageLoop;
}

namespace xwalk {
class LocationProviderTizen : public content::LocationProviderBase {
 public:
  LocationProviderTizen();
  virtual ~LocationProviderTizen();

  // LocationProvider.
  virtual bool StartProvider(bool high_accuracy) OVERRIDE;
  virtual void StopProvider() OVERRIDE;
  virtual void GetPosition(content::Geoposition* position) OVERRIDE;
  virtual void RequestRefresh() OVERRIDE;
  virtual void OnPermissionGranted() OVERRIDE;

 private:
  bool InitLocationManager();
  void NotifyLocationProvider();
  static void OnStateChanged(location_service_state_e state, void *data);

 private:
  location_manager_h manager_;
  content::Geoposition last_position_;
  base::MessageLoop* geolocation_message_loop_;
  bool is_permission_granted_;
  DISALLOW_COPY_AND_ASSIGN(LocationProviderTizen);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_GEOLOCATION_TIZEN_LOCATION_PROVIDER_TIZEN_H_
