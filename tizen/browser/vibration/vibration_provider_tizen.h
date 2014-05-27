// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_VIBRATION_VIBRATION_PROVIDER_TIZEN_H_
#define XWALK_TIZEN_BROWSER_VIBRATION_VIBRATION_PROVIDER_TIZEN_H_

#include <haptic.h>
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/vibration_provider.h"

namespace xwalk {

class VibrationProviderTizen : public content::VibrationProvider {
 public:
  VibrationProviderTizen();

 private:
  virtual ~VibrationProviderTizen();
  void InitializeHapticDevice();

  // VibrationProvider
  virtual void Vibrate(int64 milliseconds) OVERRIDE;
  virtual void CancelVibration() OVERRIDE;

  haptic_device_h handle_;
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_BROWSER_VIBRATION_VIBRATION_PROVIDER_TIZEN_H_
