// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_MOBILE_SENSOR_TIZEN_DATA_FETCHER_SHARED_MEMORY_H_
#define XWALK_TIZEN_MOBILE_SENSOR_TIZEN_DATA_FETCHER_SHARED_MEMORY_H_

#include "content/browser/device_sensors/data_fetcher_shared_memory.h"
#include "xwalk/tizen/mobile/sensor/sensor_provider.h"

namespace xwalk {

// This class receives sensor data from SensorProvider, and put them into
// a block of memory which is shared between xwalk and renderer processes.
class TizenDataFetcherSharedMemory : public content::DataFetcherSharedMemory,
                                     public SensorProvider::Observer {
 public:
  TizenDataFetcherSharedMemory();
  virtual ~TizenDataFetcherSharedMemory();

 private:
  // From content::DataFetcherSharedMemory
  bool Start(content::ConsumerType type, void* buffer) override;
  bool Stop(content::ConsumerType type) override;

  // From SensorProvider::Observer
  void OnOrientationChanged(float alpha,
                            float beta,
                            float roll) override;
  void OnAccelerationChanged(float raw_x, float raw_y, float raw_z,
                             float x, float y, float z) override;
  void OnRotationRateChanged(float alpha,
                             float beta,
                             float roll) override;

  content::DeviceMotionHardwareBuffer* motion_buffer_;
  content::DeviceOrientationHardwareBuffer* orientation_buffer_;

  DISALLOW_COPY_AND_ASSIGN(TizenDataFetcherSharedMemory);
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_MOBILE_SENSOR_TIZEN_DATA_FETCHER_SHARED_MEMORY_H_
