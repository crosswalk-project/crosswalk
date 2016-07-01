// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DISPLAY_INFO_PROVIDER_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DISPLAY_INFO_PROVIDER_H_

#include <memory>

#include "base/observer_list.h"
#include "ui/display/display_observer.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

namespace xwalk {
namespace sysapps {

using jsapi::device_capabilities::DisplayUnit;
using jsapi::device_capabilities::SystemDisplay;

class DisplayInfoProvider : public display::DisplayObserver {
 public:
  DisplayInfoProvider();
  ~DisplayInfoProvider() override;

  static std::unique_ptr<SystemDisplay> display_info();

  class Observer {
   public:
    Observer() {}
    virtual ~Observer() {}

    virtual void OnDisplayConnected(const DisplayUnit& display) = 0;
    virtual void OnDisplayDisconnected(const DisplayUnit& display) = 0;
  };

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);
  bool HasObserver(Observer* observer) const;

 private:
  void StartDisplayMonitoring();
  void StopDisplayMonitoring();

  // display::DisplayObserver implementation.
  void OnDisplayMetricsChanged(const display::Display& display,
                                       uint32_t metrics) override {}
  void OnDisplayAdded(const display::Display& display) override;
  void OnDisplayRemoved(const display::Display& display) override;

  base::ObserverList<Observer> observer_list_;

  DISALLOW_COPY_AND_ASSIGN(DisplayInfoProvider);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DISPLAY_INFO_PROVIDER_H_
