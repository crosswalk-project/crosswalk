// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/ui/date_time_chooser_tizen.h"

#include "base/logging.h"
#include "content/common/view_messages.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"

#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_system_linux.h"
#include "xwalk/dbus/dbus_manager.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

using content::BrowserThread;

namespace xwalk {
namespace {
const char kDialogLauncherServiceName[] = "org.crosswalkproject.DialogLauncher";
const char kDialogLauncherInterfaceName[] =
                                          "org.crosswalkproject.DialogLauncher";
const char kDialogLauncherObjectPath[] = "/org/crosswalkproject/DialogLauncher";
const char kOpenDateTimeDialogMethod[] = "OpenDateTimeDialog";
const char kDateTimeDialogClosedSignal[] = "DateTimeDialogClosed";
const char kDateTimeDialogCanceledSignal[] = "DateTimeDialogCanceled";
}

DateTimeChooserTizen::DateTimeChooserTizen(content::WebContents* web_contents)
  : content::WebContentsObserver(web_contents) {
  XWalkRunner* runner = XWalkRunner::GetInstance();
  application::ApplicationSystemLinux* app_system =
     static_cast<application::ApplicationSystemLinux*>(
         runner->app_system());

  session_bus_ = app_system->dbus_manager().session_bus();
  dialog_launcher_proxy_ =
      session_bus_->GetObjectProxy(kDialogLauncherServiceName,
                                   dbus::ObjectPath(kDialogLauncherObjectPath));

  if (!dialog_launcher_proxy_) {
    LOG(ERROR) << "Cannot initialize dialog launcher proxy object";
    return;
  }

  dialog_launcher_proxy_->ConnectToSignal(kDialogLauncherInterfaceName,
                                          kDateTimeDialogClosedSignal,
  base::Bind(&DateTimeChooserTizen::OnDialogClosedSignal,
             base::Unretained(this)),
  base::Bind(&DateTimeChooserTizen::OnDialogClosedSignalConnected,
             base::Unretained(this)));
}

DateTimeChooserTizen::~DateTimeChooserTizen() {
}

void DateTimeChooserTizen::ShowDialog(int type,
                                      int year,
                                      int month,
                                      int day,
                                      int hour,
                                      int minute,
                                      int second,
                                      int millisecond,
                                      int week,
                                      double min,
                                      double max,
                                      double step) {
  DateTimeChooserParams params = {routing_id(), type, year, month,
                                  day, hour, minute, second, millisecond, week,
                                  min, max, step};

  base::Closure task = base::Bind(&DateTimeChooserTizen::ShowDialogWithParams,
                                  base::Unretained(this),
                                  params);
  session_bus_->GetOriginTaskRunner()->PostTask(FROM_HERE, task);
}

void DateTimeChooserTizen::ShowDialogWithParams(
    const DateTimeChooserParams& params) {
  if (dialog_launcher_proxy_) {
    dbus::MethodCall method_call(kDialogLauncherInterfaceName,
                                  kOpenDateTimeDialogMethod);
    dbus::MessageWriter writer(&method_call);
    writer.AppendInt32(params.routing_id);
    writer.AppendInt32(params.type);
    writer.AppendInt32(params.year);
    writer.AppendInt32(params.month);
    writer.AppendInt32(params.day);
    writer.AppendInt32(params.hour);
    writer.AppendInt32(params.minute);
    writer.AppendInt32(params.second);
    writer.AppendInt32(params.millisecond);
    writer.AppendInt32(params.week);
    writer.AppendInt32(params.min);
    writer.AppendInt32(params.max);
    writer.AppendInt32(params.step);
    dialog_launcher_proxy_->CallMethod(&method_call,
                                   dbus::ObjectProxy::TIMEOUT_USE_DEFAULT,
                                   dbus::ObjectProxy::EmptyResponseCallback());
  }
}

void DateTimeChooserTizen::OnDialogClosedSignal(dbus::Signal* signal) {
  DCHECK(signal);

  dbus::MessageReader reader(signal);
  int32 uid;
  reader.PopInt32(&uid);
  if (uid != routing_id()) {
    LOG(WARNING) << "Received DialogClosed signal with wrong routing id.";
    return;
  }

  ViewHostMsg_DateTimeDialogValue_Params value;
  reader.PopInt32(&value.dialog_type);
  reader.PopInt32(&value.year);
  reader.PopInt32(&value.month);
  reader.PopInt32(&value.day);
  reader.PopInt32(&value.hour);
  reader.PopInt32(&value.minute);
  reader.PopInt32(&value.second);
  reader.PopInt32(&value.milli);
  reader.PopInt32(&value.week);

  BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
      base::Bind(&DateTimeChooserTizen::ReplaceDateTime,
      base::Unretained(this),
      value));
}

void DateTimeChooserTizen::OnDialogClosedSignalConnected(
    const std::string& interface,
    const std::string& signal,
    bool success) {
  if (!success)
    LOG(ERROR) << "Cannot connect to: " << signal
               << " on interface: " << interface;
}

bool DateTimeChooserTizen::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(DateTimeChooserTizen, message)
  IPC_MESSAGE_HANDLER(ViewHostMsg_OpenDateTimeDialog,
                      OnOpenDateTimeDialog)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void DateTimeChooserTizen::WebContentsDestroyed(
    content::WebContents* web_contents) {
  delete this;
}

void DateTimeChooserTizen::OnOpenDateTimeDialog(
    const ViewHostMsg_DateTimeDialogValue_Params& value) {
  ShowDialog(value.dialog_type,
             value.year,
             value.month,
             value.day,
             value.hour,
             value.minute,
             value.second,
             value.milli,
             value.week,
             value.minimum,
             value.maximum,
             value.step);
}

void DateTimeChooserTizen::ReplaceDateTime(
    ViewHostMsg_DateTimeDialogValue_Params value) {
  Send(new ViewMsg_ReplaceDateTime(routing_id(), value));
}

void DateTimeChooserTizen::CancelDialog() {
  Send(new ViewMsg_CancelDateTimeDialog(routing_id()));
}

}  // namespace xwalk
