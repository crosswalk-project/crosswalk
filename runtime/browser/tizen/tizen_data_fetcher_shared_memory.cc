// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>

#include "xwalk/runtime/browser/tizen/tizen_data_fetcher_shared_memory.h"

namespace xwalk {

TizenDataFetcherSharedMemory::TizenDataFetcherSharedMemory()
    : motion_buffer_(NULL),
      orientation_buffer_(NULL) {
}

TizenDataFetcherSharedMemory::~TizenDataFetcherSharedMemory() {
  Stop(content::CONSUMER_TYPE_MOTION);
  Stop(content::CONSUMER_TYPE_ORIENTATION);
}

void TizenDataFetcherSharedMemory::OnAccelerationChanged(
    float raw_x, float raw_y, float raw_z,
    float x, float y, float z) {
  if (!motion_buffer_)
    return;

  motion_buffer_->seqlock.WriteBegin();
  motion_buffer_->data.accelerationIncludingGravityX = raw_x;
  motion_buffer_->data.hasAccelerationIncludingGravityX = true;
  motion_buffer_->data.accelerationIncludingGravityY = raw_y;
  motion_buffer_->data.hasAccelerationIncludingGravityY = true;
  motion_buffer_->data.accelerationIncludingGravityZ = raw_z;
  motion_buffer_->data.hasAccelerationIncludingGravityZ = true;
  if (x != FP_NAN) {
    motion_buffer_->data.accelerationX = x;
    motion_buffer_->data.hasAccelerationX = true;
  }
  if (y != FP_NAN) {
    motion_buffer_->data.accelerationY = y;
    motion_buffer_->data.hasAccelerationY = true;
  }
  if (z != FP_NAN) {
    motion_buffer_->data.accelerationZ = z;
    motion_buffer_->data.hasAccelerationZ = true;
  }
  motion_buffer_->data.allAvailableSensorsAreActive = true;
  motion_buffer_->seqlock.WriteEnd();
}

void TizenDataFetcherSharedMemory::OnOrientationChanged(float alpha,
                                                        float beta,
                                                        float gamma) {
  if (!orientation_buffer_)
    return;

  orientation_buffer_->seqlock.WriteBegin();
  orientation_buffer_->data.alpha = alpha;
  orientation_buffer_->data.hasAlpha = true;
  orientation_buffer_->data.beta = beta;
  orientation_buffer_->data.hasBeta = true;
  orientation_buffer_->data.gamma = gamma;
  orientation_buffer_->data.hasGamma = true;
  orientation_buffer_->data.allAvailableSensorsAreActive = true;
  orientation_buffer_->seqlock.WriteEnd();
}

void TizenDataFetcherSharedMemory::OnRotationRateChanged(float alpha,
                                                         float beta,
                                                         float gamma) {
  if (!motion_buffer_)
    return;

  motion_buffer_->seqlock.WriteBegin();
  motion_buffer_->data.rotationRateAlpha = alpha;
  motion_buffer_->data.hasRotationRateAlpha = true;
  motion_buffer_->data.rotationRateBeta = beta;
  motion_buffer_->data.hasRotationRateBeta = true;
  motion_buffer_->data.rotationRateGamma = gamma;
  motion_buffer_->data.hasRotationRateGamma = true;
  motion_buffer_->data.allAvailableSensorsAreActive = true;
  motion_buffer_->seqlock.WriteEnd();
}

bool TizenDataFetcherSharedMemory::Start(content::ConsumerType type,
                                         void* buffer) {
  DCHECK(buffer);

  bool started = (motion_buffer_ || orientation_buffer_);
  switch (type) {
    case content::CONSUMER_TYPE_MOTION:
      motion_buffer_ =
          static_cast<content::DeviceMotionHardwareBuffer*>(buffer);
      break;
    case content::CONSUMER_TYPE_ORIENTATION:
      orientation_buffer_ =
          static_cast<content::DeviceOrientationHardwareBuffer*>(buffer);
      break;
    default:
      NOTREACHED();
      return false;
  }

  if (!started && SensorProvider::GetInstance())
    SensorProvider::GetInstance()->AddObserver(this);

  return true;
}

bool TizenDataFetcherSharedMemory::Stop(content::ConsumerType type) {
  switch (type) {
    case content::CONSUMER_TYPE_MOTION:
      if (motion_buffer_) {
        motion_buffer_->seqlock.WriteBegin();
        motion_buffer_->data.allAvailableSensorsAreActive = false;
        motion_buffer_->seqlock.WriteEnd();
        motion_buffer_ = NULL;
      }
      break;
    case content::CONSUMER_TYPE_ORIENTATION:
      if (orientation_buffer_) {
        orientation_buffer_->seqlock.WriteBegin();
        orientation_buffer_->data.allAvailableSensorsAreActive = false;
        orientation_buffer_->seqlock.WriteEnd();
        orientation_buffer_ = NULL;
      }
      break;
    default:
      NOTREACHED();
      return false;
  }

  if (!motion_buffer_ && !orientation_buffer_ &&
      SensorProvider::GetInstance())
    SensorProvider::GetInstance()->RemoveObserver(this);

  return true;
}

}  // namespace xwalk
