// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_APP_TIZEN_MESSAGE_PUMP_EFL_H_
#define XWALK_RUNTIME_APP_TIZEN_MESSAGE_PUMP_EFL_H_

#include "base/base_export.h"
#include "base/event_types.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_pump_aurax11.h"
#include "base/time.h"

namespace base {

class MessagePumpEFL : public MessagePumpAuraX11 {
 public:
  MessagePumpEFL();

  // Like MessagePump::Run, but events are routed through dispatcher.
  virtual void RunWithDispatcher(Delegate* delegate,
                                 MessagePumpDispatcher* dispatcher) OVERRIDE;

  void HandleWakeUp();
  bool HandleDispatch();

  // Overridden from MessagePump:
  virtual void Run(Delegate* delegate) OVERRIDE;
  virtual void Quit() OVERRIDE;
  virtual void ScheduleWork() OVERRIDE;
  virtual void ScheduleDelayedWork(
      const TimeTicks& delayed_work_time) OVERRIDE;

 protected:
  virtual ~MessagePumpEFL();

 private:
  bool DoWorks();

  struct Private;
  scoped_ptr<Private> private_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpEFL);
};

}  // namespace base

#endif  // XWALK_RUNTIME_APP_TIZEN_MESSAGE_PUMP_EFL_H_
