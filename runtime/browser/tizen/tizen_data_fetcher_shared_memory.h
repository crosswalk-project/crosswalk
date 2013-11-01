// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_DATA_FETCHER_SHARED_MEMORY_H_
#define XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_DATA_FETCHER_SHARED_MEMORY_H_

#include "content/browser/device_orientation/data_fetcher_shared_memory.h"
#include "xwalk/runtime/browser/tizen/sensor_provider.h"

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
  virtual bool Start(content::ConsumerType type, void* buffer) OVERRIDE;
  virtual bool Stop(content::ConsumerType type) OVERRIDE;

  // From SensorProvider::Observer
  virtual void OnOrientationChanged(float alpha,
                                    float beta,
                                    float roll) OVERRIDE;
  virtual void OnAccelerationChanged(float raw_x, float raw_y, float raw_z,
                                     float x, float y, float z) OVERRIDE;
  virtual void OnRotationRateChanged(float alpha,
                                     float beta,
                                     float roll) OVERRIDE;

  content::DeviceMotionHardwareBuffer* motion_buffer_;
  content::DeviceOrientationHardwareBuffer* orientation_buffer_;

  DISALLOW_COPY_AND_ASSIGN(TizenDataFetcherSharedMemory);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_TIZEN_TIZEN_DATA_FETCHER_SHARED_MEMORY_H_
