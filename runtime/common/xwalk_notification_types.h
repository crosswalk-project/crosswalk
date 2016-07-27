// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_NOTIFICATION_TYPES_H_
#define XWALK_RUNTIME_COMMON_XWALK_NOTIFICATION_TYPES_H_

#include "build/build_config.h"
#include "content/public/browser/notification_types.h"

namespace xwalk {

enum NotificationType {
  NOTIFICATION_XWALK_START = content::NOTIFICATION_CONTENT_END,

  // Notify that fullscreen state of a NativeAppWindow is changed.
  NOTIFICATION_FULLSCREEN_CHANGED = NOTIFICATION_XWALK_START,

  // Printing ----------------------------------------------------------------

  // Notification from PrintJob that an event occurred. It can be that a page
  // finished printing or that the print job failed. Details is
  // PrintJob::EventDetails. Source is a PrintJob.
  NOTIFICATION_XWALK_PRINT_JOB_EVENT,

  // Sent when a PrintJob has been released.
  // Source is the WebContents that holds the print job.
  NOTIFICATION_XWALK_PRINT_JOB_RELEASED,

  NOTIFICATION_XWALK_END,
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_COMMON_XWALK_NOTIFICATION_TYPES_H_
