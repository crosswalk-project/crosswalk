// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/tizen/message_pump_efl.h"

#include <Ecore.h>
#include <Elementary.h>
#include "base/debug/trace_event.h"
#include "base/logging.h"
#include "base/posix/eintr_wrapper.h"
#include "base/run_loop.h"
#include "base/threading/platform_thread.h"

namespace {

const int kPipeMessageSize = 1;
const char kWakupPipeMessage[] = "W";

void WakeUpEvent(void* data, void*, unsigned int) {
  static_cast<base::MessagePumpEFL*>(data)->HandleWakeUp();
}

Eina_Bool ScheduleWorks(void* data) {
  return static_cast<base::MessagePumpEFL*>(data)->HandleDispatch();
}

}  // namespace

namespace base {

struct MessagePumpEFL::Private {
  MessagePump::Delegate* delegate;

  bool should_quit;

  // This is the time when we need to do delayed work.
  TimeTicks delayed_work_time;

  // We use a wakeup pipe to make sure we'll get out of the ecore polling phase
  // when another thread has scheduled us to do some work.
  Ecore_Pipe* wakeup_pipe;

  // HandleDispatch() runs every ecore_main_loop_iterate().
  Ecore_Timer* timer;

  scoped_ptr<base::RunLoop> run_loop;
};

MessagePumpEFL::MessagePumpEFL()
    : private_(new Private) {
  private_->wakeup_pipe = ecore_pipe_add(WakeUpEvent, this);
  private_->delegate = MessageLoopForUI::current();
  private_->should_quit = false;
  private_->run_loop.reset(new base::RunLoop);
  if (!private_->run_loop->BeforeRun())
    NOTREACHED();

  // TODO(dshwang): Don't do ScheduleWorks() when there are not works.
  private_->timer = ecore_timer_add(0, ScheduleWorks, this);
}

MessagePumpEFL::~MessagePumpEFL() {
  ecore_pipe_del(private_->wakeup_pipe);
}

void MessagePumpEFL::RunWithDispatcher(Delegate* delegate,
    MessagePumpDispatcher* dispatcher) {
  NOTREACHED();
}

void MessagePumpEFL::HandleWakeUp() {
  private_->delegate->DoWork();

  if (private_->should_quit)
    return;

  private_->delegate->DoDelayedWork(&private_->delayed_work_time);
}

bool MessagePumpEFL::HandleDispatch() {
  DoWorks();

  if (private_->should_quit)
    return false;

  return true;
}

bool MessagePumpEFL::DoWorks() {
#ifndef NDEBUG
  // Make sure we only run this on one thread. X/GTK only has one message pump
  // so we can only have one UI loop per process.
  static base::PlatformThreadId thread_id = base::PlatformThread::CurrentId();
  DCHECK(thread_id == base::PlatformThread::CurrentId()) <<
      "Running MessagePumpEFL on two different threads; "
      "this is unsupported by Ecore!";
#endif

  bool more_work_is_plausible = private_->delegate->DoWork();

  more_work_is_plausible |=
      private_->delegate->DoDelayedWork(&private_->delayed_work_time);

  if (more_work_is_plausible)
    return true;

  more_work_is_plausible |= private_->delegate->DoIdleWork();

  return more_work_is_plausible;
}

void MessagePumpEFL::Run(Delegate* delegate) {
  NOTREACHED();
}

void MessagePumpEFL::Quit() {
  private_->should_quit = true;
  if (private_->run_loop) {
    private_->run_loop->AfterRun();
    private_->run_loop.reset();
  }
  elm_exit();
}

void MessagePumpEFL::ScheduleWork() {
  // This can be called on any thread, so we don't want to touch any state
  // variables as we would then need locks all over.  This ensures that if
  // we are sleeping in a poll that we will wake up.
  if (HANDLE_EINTR(ecore_pipe_write(private_->wakeup_pipe,
                                    kWakupPipeMessage,
                                    kPipeMessageSize)) != 1) {
    NOTREACHED() << "Could not write to the UI message loop wakeup pipe!";
  }
}

void MessagePumpEFL::ScheduleDelayedWork(
    const TimeTicks& delayed_work_time) {
  // We need to wake up the loop in case the poll timeout needs to be
  // adjusted.  This will cause us to try to do work, but that's ok.
  private_->delayed_work_time = delayed_work_time;
  ScheduleWork();
}

}  // namespace base
