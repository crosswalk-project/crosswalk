// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_RUNTIME_COMMON_XWALK_NOTIFICATION_TYPES_H_
#define CAMEO_RUNTIME_COMMON_XWALK_NOTIFICATION_TYPES_H_

#include "build/build_config.h"
#include "content/public/browser/notification_types.h"

namespace xwalk {

enum NotificationType {
  NOTIFICATION_XWALK_START = content::NOTIFICATION_CONTENT_END,

  // Notify that a new Runtime instance is created. The source is a
  // Source<Runtime> containing the affected Runtime. No details is provided.
  NOTIFICATION_RUNTIME_OPENED = NOTIFICATION_XWALK_START,

  // Notify that a Runtime instance is close. The source is a Source<Runtime>
  // containing the affected Runtime. No details is provided.
  NOTIFICATION_RUNTIME_CLOSED,

  // Notify that fullscreen state of a NativeAppWindow is changed.
  NOTIFICATION_FULLSCREEN_CHANGED,

  NOTIFICATION_XWALK_END,
};

}  // namespace xwalk

#endif  // CAMEO_RUNTIME_COMMON_XWALK_NOTIFICATION_TYPES_H_
