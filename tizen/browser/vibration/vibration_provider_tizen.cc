// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/vibration/vibration_provider_tizen.h"

#include "base/logging.h"
#include "content/browser/vibration/vibration_message_filter.h"
#include "content/common/view_messages.h"
#include "third_party/WebKit/public/platform/WebVibration.h"

namespace xwalk {

VibrationProviderTizen::VibrationProviderTizen()
    : handle_(NULL) {
  InitializeHapticDevice();
}

VibrationProviderTizen::~VibrationProviderTizen() {
  if (handle_)
    haptic_close(handle_);
}

void VibrationProviderTizen::InitializeHapticDevice() {
  if (haptic_open(HAPTIC_DEVICE_0, &handle_) != HAPTIC_ERROR_NONE) {
    LOG(WARNING) << "Vibration Provider for Tizen not initialized.";
  }
}

void VibrationProviderTizen::Vibrate(int64 milliseconds) {
  if (handle_)
    haptic_vibrate_monotone(handle_, milliseconds, NULL);
}

void VibrationProviderTizen::CancelVibration() {
  if (handle_)
    haptic_stop_all_effects(handle_);
}

}  // namespace xwalk

namespace content {
// static
VibrationProvider* VibrationMessageFilter::CreateProvider() {
  return new xwalk::VibrationProviderTizen();
}

}  // namespace content
