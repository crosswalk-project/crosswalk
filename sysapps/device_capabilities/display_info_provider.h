// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_SYSAPPS_DEVICE_CAPABILITIES_DISPLAY_INFO_PROVIDER_H_
#define XWALK_SYSAPPS_DEVICE_CAPABILITIES_DISPLAY_INFO_PROVIDER_H_

#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "ui/gfx/display_observer.h"
#include "xwalk/sysapps/device_capabilities/device_capabilities.h"

namespace xwalk {
namespace sysapps {

using jsapi::device_capabilities::DisplayUnit;
using jsapi::device_capabilities::SystemDisplay;

class DisplayInfoProvider : public gfx::DisplayObserver {
 public:
  DisplayInfoProvider();
  virtual ~DisplayInfoProvider();

  static scoped_ptr<SystemDisplay> display_info();

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

  // gfx::DisplayObserver implementation.
  virtual void OnDisplayBoundsChanged(const gfx::Display& display) OVERRIDE {}
  virtual void OnDisplayAdded(const gfx::Display& display) OVERRIDE;
  virtual void OnDisplayRemoved(const gfx::Display& display) OVERRIDE;

  ObserverList<Observer> observer_list_;

  DISALLOW_COPY_AND_ASSIGN(DisplayInfoProvider);
};

}  // namespace sysapps
}  // namespace xwalk

#endif  // XWALK_SYSAPPS_DEVICE_CAPABILITIES_DISPLAY_INFO_PROVIDER_H_
