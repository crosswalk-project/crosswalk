// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_UI_DATE_TIME_CHOOSER_TIZEN_H_
#define XWALK_TIZEN_BROWSER_UI_DATE_TIME_CHOOSER_TIZEN_H_

#include <string>
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_observer.h"

namespace dbus {
class ObjectProxy;
class Signal;
class Bus;
}

namespace content {
class WebContents;
}

struct ViewHostMsg_DateTimeDialogValue_Params;

namespace xwalk {
class DateTimeChooserTizen : public content::WebContentsObserver {
 public:
  struct DateTimeChooserParams {
    int routing_id;
    int type;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
    int week;
    double min;
    double max;
    double step;
  };

 public:
  explicit DateTimeChooserTizen(content::WebContents* web_contents);
  ~DateTimeChooserTizen();

 private:
  void Init();
  void ShowDialog(int type,
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
                  double step);

  void ShowDialogWithParams(const DateTimeChooserParams& params);
  void OnDialogClosedSignal(dbus::Signal* signal);
  void OnDialogClosedSignalConnected(const std::string&,
                                     const std::string&,
                                     bool);

  // WebContents observer implementation
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void WebContentsDestroyed(
      content::WebContents* web_contents) OVERRIDE;
  void OnOpenDateTimeDialog(
      const ViewHostMsg_DateTimeDialogValue_Params& param);

  void ReplaceDateTime(ViewHostMsg_DateTimeDialogValue_Params value);
  void CancelDialog();

 private:
  scoped_refptr<dbus::Bus> session_bus_;
  // Deleted by bus
  dbus::ObjectProxy* dialog_launcher_proxy_;

  DISALLOW_COPY_AND_ASSIGN(DateTimeChooserTizen);
};

}  // namespace xwalk

#endif  // XWALK_TIZEN_BROWSER_UI_DATE_TIME_CHOOSER_TIZEN_H_
