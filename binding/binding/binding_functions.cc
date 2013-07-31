/* Copyright (c) 2013 Intel Corporation. All rights reserved.
   Use of this source code is governed by a BSD-style license that can be
   found in the LICENSE file. */

#include "xwalk/binding/binding/binding_functions.h"

#include <map>

#include "content/common/child_process.h"
#include "content/common/child_thread.h"
#include "content/common/content_param_traits.h"
#include "content/common/np_channel_base.h"
#include "content/common/npobject_proxy.h"
#include "content/common/npobject_util.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebBindings.h"
#include "v8/include/v8.h"
#include "webkit/plugins/npapi/plugin_host.h"
#include "xwalk/binding/common/binding_messages.h"
#include "xwalk/binding/public/xwalk_binding.h"

namespace {

class BindingTimer {
 public:
  BindingTimer() : next_timer_id_(1) {}
  ~BindingTimer() {}

  uint32_t ScheduleTimer(uint32 interval,
                         bool repeat,
                         void (*func)(NPP id, uint32 timer_id));
  void UnscheduleTimer(uint32 timer_id);

 private:
  void OnTimerCall(void (*func)(NPP id, uint32 timer_id),
                   NPP id,
                   uint32 timer_id);

  // Next unusued timer id.
  uint32 next_timer_id_;

  // Map of timer id to settings for timer.
  struct TimerInfo {
    uint32 interval;
    bool repeat;
  };
  typedef std::map<uint32, TimerInfo> TimerMap;
  TimerMap timers_;

  DISALLOW_COPY_AND_ASSIGN(BindingTimer);
};

uint32_t BindingTimer::ScheduleTimer(uint32 interval,
                                     bool repeat,
                                     void (*func)(NPP id, uint32 timer_id)) {
  // Use next timer id.
  uint32 timer_id;
  timer_id = next_timer_id_;
  ++next_timer_id_;

  // Record timer interval and repeat.
  TimerInfo info;
  info.interval = interval;
  info.repeat = repeat ? true : false;
  timers_[timer_id] = info;

  // Schedule the callback.
  base::MessageLoop::current()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&BindingTimer::OnTimerCall,
                 base::Unretained(this),
                 func,
                 reinterpret_cast<NPP>(NULL),
                 timer_id),
      base::TimeDelta::FromMilliseconds(interval));
  return timer_id;
}

void BindingTimer::UnscheduleTimer(uint32 timer_id) {
  // Remove info about the timer.
  TimerMap::iterator it = timers_.find(timer_id);
  if (it != timers_.end())
    timers_.erase(it);
}

void BindingTimer::OnTimerCall(void (*func)(NPP id, uint32 timer_id),
                               NPP id,
                               uint32 timer_id) {
  // Do not invoke callback if the timer has been unscheduled.
  TimerMap::iterator it = timers_.find(timer_id);
  if (it == timers_.end())
    return;

  // Get all information about the timer before invoking the callback. The
  // callback might unschedule the timer.
  TimerInfo info = it->second;

  func(id, timer_id);

  // If the timer was unscheduled by the callback, just free up the timer id.
  if (timers_.find(timer_id) == timers_.end())
    return;

  // Reschedule repeating timers after invoking the callback so callback is not
  // re-entered if it pumps the message loop.
  if (info.repeat) {
    base::MessageLoop::current()->PostDelayedTask(
        FROM_HERE,
        base::Bind(&BindingTimer::OnTimerCall,
                   base::Unretained(this),
                   func,
                   reinterpret_cast<NPP>(NULL),
                   timer_id),
        base::TimeDelta::FromMilliseconds(info.interval));
  } else {
    timers_.erase(it);
  }
}

static BindingTimer timer;

uint32_t Binding_ScheduleTimer(NPP npp,
                               uint32 interval,
                               bool repeat,
                               void (*func)(NPP id, uint32 timer_id)) {
  return timer.ScheduleTimer(interval, repeat, func);
}

void Binding_UnscheduleTimer(NPP npp, uint32 timer_id) {
  return timer.UnscheduleTimer(timer_id);
}

void Binding_ThreadAsyncCall(NPP npp, void (*func)(void*), void* user_data) {
  content::ChildProcess::current()->main_thread()->message_loop()->PostTask(
      FROM_HERE, base::Bind(func, user_data));
}

void Binding_SetException(NPObject* npobj, const NPVariant* value) {
  if (!content::IsPluginProcess()) {
    if (NPVARIANT_IS_STRING(*value)) {
      const char* msg = NPVARIANT_TO_STRING(*value).UTF8Characters;
      WebKit::WebBindings::setException(npobj, msg);
    } else {
      v8::Handle<v8::Value> ex = WebKit::WebBindings::toV8Value(value);
      v8::ThrowException(ex);
    }
  } else {
    content::NPChannelBase* renderer_channel =
        content::NPChannelBase::GetCurrentChannel();
    if (renderer_channel) {
      content::NPVariant_Param value_param;
      int route_id = MSG_ROUTING_NONE;
      content::CreateNPVariantParam(*value, renderer_channel, &value_param,
                                    false, 0, GURL());
      if (npobj->_class != content::NPObjectProxy::npclass())
        route_id = renderer_channel->GetExistingRouteForNPObjectStub(npobj);
      renderer_channel->Send(
          new BindingHostMsg_SetException(route_id, value_param));
    }
  }
}

XWalkBindingFunctions Binding_Functions;

}  // namespace


const XWalkBindingFunctions* GetBindingFunctions() {
  if (Binding_Functions.size == sizeof(Binding_Functions))
    return &Binding_Functions;

  // Initialize host functions table
  NPNetscapeFuncs* funcs =
      webkit::npapi::PluginHost::Singleton()->host_functions();
  Binding_Functions.size                   = sizeof(Binding_Functions);
  Binding_Functions.version                = XWALK_BINDING_VERSION;
  Binding_Functions.memalloc               = funcs->memalloc;
  Binding_Functions.memfree                = funcs->memfree;
  Binding_Functions.memflush               = funcs->memflush;
  Binding_Functions.getstringidentifier    = funcs->getstringidentifier;
  Binding_Functions.getstringidentifiers   = funcs->getstringidentifiers;
  Binding_Functions.getintidentifier       = funcs->getintidentifier;
  Binding_Functions.identifierisstring     = funcs->identifierisstring;
  Binding_Functions.utf8fromidentifier     = funcs->utf8fromidentifier;
  Binding_Functions.intfromidentifier      = funcs->intfromidentifier;
  Binding_Functions.createobject           = funcs->createobject;
  Binding_Functions.retainobject           = funcs->retainobject;
  Binding_Functions.releaseobject          = funcs->releaseobject;
  Binding_Functions.invoke                 = funcs->invoke;
  Binding_Functions.invokeDefault          = funcs->invokeDefault;
  Binding_Functions.evaluate               = funcs->evaluate;
  Binding_Functions.getproperty            = funcs->getproperty;
  Binding_Functions.setproperty            = funcs->setproperty;
  Binding_Functions.removeproperty         = funcs->removeproperty;
  Binding_Functions.hasproperty            = funcs->hasproperty;
  Binding_Functions.hasmethod              = funcs->hasmethod;
  Binding_Functions.releasevariantvalue    = funcs->releasevariantvalue;
  Binding_Functions.setexception           = Binding_SetException;
  Binding_Functions.enumerate              = funcs->enumerate;
  Binding_Functions.bindingthreadasynccall = Binding_ThreadAsyncCall;
  Binding_Functions.construct              = funcs->construct;
  Binding_Functions.scheduletimer          = Binding_ScheduleTimer;
  Binding_Functions.unscheduletimer        = Binding_UnscheduleTimer;
  return &Binding_Functions;
}
