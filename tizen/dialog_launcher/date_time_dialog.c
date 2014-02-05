// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>

#include <Elementary.h>
#include <Ecore.h>
#include <E_DBus.h>

#include "date_time_dialog.h"
#include "dialog_launcher.h"

E_DBus_Connection* conn;

const int first_day_of_month[] =
                      { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
const int first_day_of_month_leap[] =
                      { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

/**
 * OpenDateTimeDialog method signature
 * i - uid
 * i - dialog type
 * i - year
 * i - month
 * i - day
 * i - hour
 * i - minute
 * i - second
 * i - millisecond
 * i - week
 * i - minimum
 * i - maximum
 * i - step
 */
const char kOpenDateTimeDialogMethod[] = "OpenDateTimeDialog";
const char kOpenDateTimeDialogMethodSignature[] = "iiiiiiiiiiiii";

/**
 * DateTimeDialogClosed signal signature
 * i - uid
 * i - dialog type
 * i - year
 * i - month
 * i - day
 * i - hour
 * i - minute
 * i - second
 * i - millisecond
 * i - week
 */
const char kDateTimeDialogClosedSignal[] = "DateTimeDialogClosed";
const char kDateTimeDialogClosedSignalSignature[] = "iiiiiiiiii";

/**
 * DateTimeDialogCanceled signal signature
 * i - uid
 */
const char kDateTimeDialogCanceledSignal[] = "DateTimeDialogCanceled";
const char kDateTimeDialogCanceledSignalSignature[] = "i";

typedef struct DateTimeDialogParams DateTimeDialogParams;
typedef struct DateTimeDialog DateTimeDialog;
typedef enum DateTimeDialogType DateTimeDialogType;

enum DateTimeDialogType {
  DATE_TIME_DIALOG_TYPE_DATE = 8,
  DATE_TIME_DIALOG_TYPE_DATE_TIME,
  DATE_TIME_DIALOG_TYPE_DATE_TIME_LOCAL,
  DATE_TIME_DIALOG_TYPE_MONTH,
  DATE_TIME_DIALOG_TYPE_TIME,
  DATE_TIME_DIALOG_TYPE_WEEK
};

struct DateTimeDialog {
  DateTimeDialogType date_time_dialog_type;
  Evas_Object* elm_datetime_widget;
};

struct DateTimeDialogParams {
  dbus_int32_t uid;
  dbus_int32_t type;
  dbus_int32_t year;
  dbus_int32_t month;
  dbus_int32_t day;
  dbus_int32_t hour;
  dbus_int32_t minute;
  dbus_int32_t second;
  dbus_int32_t millisecond;
  dbus_int32_t week;
  dbus_int32_t minimum;
  dbus_int32_t maximum;
  dbus_int32_t step;
};

void register_date_time_launcher(E_DBus_Connection* connection,
                                 E_DBus_Interface* interface);
static DBusMessage* open_date_time_dialog_cb(E_DBus_Object *obj,
                                             DBusMessage *msg);
static void open_datetime_dialog(DateTimeDialogParams params);
static void init_date_time_widget(DateTimeDialogType type,
                                  Evas_Object* datetime);
static void set_date_time_widget_time(DateTimeDialogParams params,
                               Evas_Object* datetime);
static void set_time_value(int* out, int in);
static void date_time_dialog_closed_cb(void *data,
                                       Evas_Object *obj,
                                       void *event_info);
static void cancel_datetime_dialog();
static void send_datetime_dialog_canceled_signal();
static Eina_Bool is_leap_year(int year);
static int week_of_the_year(int day, int month, int year);
static int day_of_the_week(int day, int month, int year);

void register_date_time_launcher(E_DBus_Connection* connection,
                                 E_DBus_Interface* interface) {
  conn = connection;
  e_dbus_interface_method_add(interface,
                              kOpenDateTimeDialogMethod,
                              kOpenDateTimeDialogMethodSignature,
                              NULL,
                              open_date_time_dialog_cb);

  e_dbus_interface_signal_add(interface,
                              kDateTimeDialogClosedSignal,
                              kDateTimeDialogClosedSignalSignature);
}

static DBusMessage* open_date_time_dialog_cb(E_DBus_Object *obj,
                                             DBusMessage *msg) {
  DBusError err;
  dbus_error_init(&err);
  DateTimeDialogParams params;
  dbus_message_get_args(msg, &err,
                        DBUS_TYPE_INT32, &params.uid,
                        DBUS_TYPE_INT32, &params.type,
                        DBUS_TYPE_INT32, &params.year,
                        DBUS_TYPE_INT32, &params.month,
                        DBUS_TYPE_INT32, &params.day,
                        DBUS_TYPE_INT32, &params.hour,
                        DBUS_TYPE_INT32, &params.minute,
                        DBUS_TYPE_INT32, &params.second,
                        DBUS_TYPE_INT32, &params.millisecond,
                        DBUS_TYPE_INT32, &params.week,
                        DBUS_TYPE_INT32, &params.maximum,
                        DBUS_TYPE_INT32, &params.minimum,
                        DBUS_TYPE_INT32, &params.step,
                        DBUS_TYPE_INVALID);

  if (!dbus_error_is_set(&err)) {
    open_datetime_dialog(params);
  } else {
    EINA_LOG_ERR("%s",
                 "Cannot get OpenDateTimeDialog parameters from dbus message.");
  }

  return dbus_message_new_method_return(msg);
}

static void open_datetime_dialog(DateTimeDialogParams params) {
  OpenedDialog* openedDialog = malloc(sizeof(OpenedDialog));
  openedDialog->uid = params.uid;
  openedDialog->type = DIALOG_TYPE_DATE_TIME;

  DateTimeDialog* dateTimeDialog = malloc(sizeof(DateTimeDialog));
  openedDialog->dialog = (void*)dateTimeDialog;
  dateTimeDialog->date_time_dialog_type = params.type;

  openedDialog->window = elm_win_add(NULL, "", ELM_WIN_BASIC);
  elm_win_borderless_set(openedDialog->window, EINA_TRUE);
  elm_win_alpha_set(openedDialog->window, EINA_TRUE);
  elm_win_autodel_set(openedDialog->window, EINA_FALSE);

  Evas_Object *popup = elm_popup_add(openedDialog->window);
  elm_object_part_text_set(popup, "title,text", "Select date");

  Evas_Object *layout = elm_box_add(popup);
  dateTimeDialog->elm_datetime_widget = elm_datetime_add(layout);
  evas_object_size_hint_align_set(dateTimeDialog->elm_datetime_widget, 0, 0);

  init_date_time_widget(params.type, dateTimeDialog->elm_datetime_widget);
  set_date_time_widget_time(params, dateTimeDialog->elm_datetime_widget);

  Evas_Object *btn = elm_button_add(popup);
  elm_button_autorepeat_set(btn, EINA_FALSE);
  elm_object_text_set(btn,"Ok");
  evas_object_smart_callback_add(btn, "clicked",
                                 date_time_dialog_closed_cb, openedDialog);

  elm_box_pack_end(layout, dateTimeDialog->elm_datetime_widget);
  elm_box_pack_end(layout, btn);
  elm_object_content_set(popup, layout);
  elm_win_resize_object_add(openedDialog->window, popup);

  evas_object_show(btn);
  evas_object_show(popup);
  evas_object_show(layout);
  evas_object_show(dateTimeDialog->elm_datetime_widget);
  evas_object_show(openedDialog->window);
}

static void init_date_time_widget(DateTimeDialogType type, Evas_Object* datetime) {
  EINA_SAFETY_ON_NULL_RETURN(datetime);
  switch(type) {
  case DATE_TIME_DIALOG_TYPE_DATE:
  case DATE_TIME_DIALOG_TYPE_WEEK:
    elm_datetime_field_visible_set(datetime, ELM_DATETIME_HOUR, EINA_FALSE);
    elm_datetime_field_visible_set(datetime, ELM_DATETIME_MINUTE, EINA_FALSE);
    elm_datetime_field_visible_set(datetime, ELM_DATETIME_AMPM, EINA_FALSE);
    break;
  case DATE_TIME_DIALOG_TYPE_DATE_TIME_LOCAL:
    // TODO(shalamov): check how to handle that.
    break;
  case DATE_TIME_DIALOG_TYPE_MONTH:
    elm_datetime_format_set(datetime, "%Y -%m");
    break;
  case DATE_TIME_DIALOG_TYPE_TIME:
    elm_datetime_format_set(datetime, "%H:%M");
    break;
  default:
    break;
  }
}

static void set_date_time_widget_time(DateTimeDialogParams params,
                                      Evas_Object* datetime) {
  EINA_SAFETY_ON_NULL_RETURN(datetime);
  time_t current_time;
  time(&current_time);
  struct tm* local_time = localtime(&current_time);
  set_time_value(&local_time->tm_year, params.year - 1900);
  set_time_value(&local_time->tm_mon, params.month);
  set_time_value(&local_time->tm_mday, params.day);
  set_time_value(&local_time->tm_hour, params.hour);
  set_time_value(&local_time->tm_min, params.minute);
  set_time_value(&local_time->tm_sec, params.second);
  elm_datetime_value_set(datetime, local_time);
}

static void set_time_value(int* out, int in) {
  EINA_SAFETY_ON_NULL_RETURN(out);
  if(in > 0)
    *out = in;
}

static void date_time_dialog_closed_cb(void *data, Evas_Object *obj,
                                       void *event_info) {
  OpenedDialog* opened_dialog = (OpenedDialog*)data;
  DateTimeDialog* date_time_dialog = (DateTimeDialog*)opened_dialog->dialog;

  struct tm selected_date;
  Eina_Bool success =
      elm_datetime_value_get(date_time_dialog->elm_datetime_widget,
                             &selected_date);

  if (success) {
    DBusMessage *msg = dbus_message_new_signal(kDialogLauncherObjectPath,
                                               kDialogLauncherInterfaceName,
                                               kDateTimeDialogClosedSignal);

    dbus_int32_t uid = opened_dialog->uid;
    dbus_int32_t type = date_time_dialog->date_time_dialog_type;
    dbus_int32_t year = selected_date.tm_year + 1900;
    dbus_int32_t month = selected_date.tm_mon;
    dbus_int32_t day = selected_date.tm_mday;
    dbus_int32_t hour = selected_date.tm_hour;
    dbus_int32_t minute = selected_date.tm_min;
    dbus_int32_t second = selected_date.tm_sec;
    dbus_int32_t millisecond = 0;
    dbus_int32_t week = week_of_the_year(day, month, year);

    dbus_message_append_args(msg, DBUS_TYPE_INT32, &uid,
                                  DBUS_TYPE_INT32, &type,
                                  DBUS_TYPE_INT32, &year,
                                  DBUS_TYPE_INT32, &month,
                                  DBUS_TYPE_INT32, &day,
                                  DBUS_TYPE_INT32, &hour,
                                  DBUS_TYPE_INT32, &minute,
                                  DBUS_TYPE_INT32, &second,
                                  DBUS_TYPE_INT32, &millisecond,
                                  DBUS_TYPE_INT32, &week,
                                  DBUS_TYPE_INVALID);

    e_dbus_message_send(conn, msg, NULL, -1, NULL);
    dbus_message_unref(msg);
  } else {
    EINA_LOG_ERR("%s","Cannot get selected date.");
  }

  free(opened_dialog->dialog);
  evas_object_del(opened_dialog->window);
  free(opened_dialog);
}

static void cancel_datetime_dialog() {
  // TODO(shalamov): not implemented
}

static void send_datetime_dialog_canceled_signal() {
  // TODO(shalamov): not implemented
}

static Eina_Bool is_leap_year(int year) {
  return ((!(year % 4) && (year % 100)) || !(year % 400));
}

static int week_of_the_year(int day, int month, int year) {
  int first_day;
  if (is_leap_year(year)) {
    first_day = first_day_of_month_leap[month-1];
  } else {
    first_day = first_day_of_month[month-1];
  }

  int ordinal_day = first_day + day;
  int weekday = day_of_the_week(day, month, year);
  return (ordinal_day - weekday + 10) / 7;
}

static int day_of_the_week(int day, int month, int year) {
  // Gaussian algorithm
  int y = year;
  int m = month;
  if (month < 3) {
    y -= 1;
    m += 10;
  } else {
    m = (m - 2) % 12;
  }

  int CC = y / 100;
  int YY = y % 100;

  int y1 = 5 * (YY % 4);
  int y2 = 3 * (YY % 7);
  int c = 5 * (CC % 4);

  return (day + abs(((26*m-2)/10)) + y1 + y2 + c) % 7;
}
