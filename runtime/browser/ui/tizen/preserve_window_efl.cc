// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/tizen/preserve_window_efl.h"

#include <Ecore_X.h>
#include <Elementary.h>
#include <X11/Xlib.h>
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"

namespace views {

namespace {
#define evas_smart_preserve_window_type "Evas_Smart_Preserve_Window"

const Evas_Smart_Cb_Description g_smart_callbacks[] = {
    {NULL, NULL}};

struct PreserveWindowData {
  Evas_Object_Smart_Clipped_Data base;
  Display* xdisplay;
  ::Window xwindow;
};

bool IsPreserveWindowEvasObject(const Evas_Object* evas_object) {
  DCHECK(evas_object);

  const char* evas_object_type = evas_object_type_get(evas_object);
  if (!evas_object_smart_type_check(evas_object,
                                    evas_smart_preserve_window_type)) {
    LOG(ERROR) << evas_object << " is not of an "<< evas_object_type << "!";
    return false;
  }

  const Evas_Smart* evas_smart = evas_object_smart_smart_get(evas_object);
  if (!evas_smart) {
    LOG(ERROR) << evas_object << "(" << evas_object_type <<
        ") is not a smart object!";
    return false;
  }

  const Evas_Smart_Class* smart_class = evas_smart_class_get(evas_smart);
  if (!smart_class) {
    LOG(ERROR) << evas_object << "(" << evas_object_type <<
        ") is not a smart class object!";
    return false;
  }

  return true;
}

inline PreserveWindowData* ToSmartData(Evas_Object* evas_object) {
  DCHECK(evas_object);
  DCHECK(IsPreserveWindowEvasObject(evas_object));
  CHECK(evas_object_smart_data_get(evas_object));
  return static_cast<PreserveWindowData*>(
      evas_object_smart_data_get(evas_object));
}

EVAS_SMART_SUBCLASS_NEW(evas_smart_preserve_window_type,
                        evas_smart_preserve_window,
                        Evas_Smart_Class, Evas_Smart_Class,
                        evas_object_smart_clipped_class_get,
                        g_smart_callbacks);

/* create and setup a new preserve window smart object's internals */
void evas_smart_preserve_window_smart_add(Evas_Object* o) {
  // Don't use EVAS_SMART_DATA_ALLOC(o, PreserveWindowData)
  // because [-fpermissive] does not allow invalid conversion
  // from 'void*' to 'PreserveWindowData*'.
  PreserveWindowData* smart_data;
  smart_data = static_cast<PreserveWindowData*>(evas_object_smart_data_get(o));
  if (!smart_data) {
    smart_data = static_cast<PreserveWindowData*>(
        calloc(1, sizeof(PreserveWindowData)));
    if (!smart_data) {
      return;
    }
    evas_object_smart_data_set(o, smart_data);
  }

  int x, y, w, h = 0;
  evas_object_geometry_get(o, &x, &y, &w, &h);
  if (w < 1) w = 1;
  if (h < 1) h = 1;

  Ecore_X_Window root_x_window = elm_win_xwindow_get(o);
  smart_data->xdisplay = static_cast<Display*>(ecore_x_display_get());

  unsigned long attribute_mask = CWBackPixmap; // NOLINT(*)
  XSetWindowAttributes swa;
  memset(&swa, 0, sizeof(swa));
  swa.background_pixmap = None;

  smart_data->xwindow = XCreateWindow(
      smart_data->xdisplay, root_x_window,
      0, 0, w, h,
      0,               // border width
      CopyFromParent,  // depth
      InputOutput,
      CopyFromParent,  // visual
      attribute_mask,
      &swa);

  XFlush(smart_data->xdisplay);
  XMapWindow(smart_data->xdisplay, smart_data->xwindow);

  evas_smart_preserve_window_parent_sc->add(o);
}

void evas_smart_preserve_window_smart_del(Evas_Object* o) {
  PreserveWindowData* smart_data = ToSmartData(o);
  XDestroyWindow(smart_data->xdisplay, smart_data->xwindow);
  smart_data->xwindow = None;
  evas_smart_preserve_window_parent_sc->del(o);
}

void evas_smart_preserve_window_smart_show(Evas_Object* o) {
  PreserveWindowData* smart_data = ToSmartData(o);
  XMapWindow(smart_data->xdisplay, smart_data->xwindow);
  evas_smart_preserve_window_parent_sc->show(o);
}

void evas_smart_preserve_window_smart_hide(Evas_Object* o) {
  PreserveWindowData* smart_data = ToSmartData(o);
  XUnmapWindow(smart_data->xdisplay, smart_data->xwindow);
  evas_smart_preserve_window_parent_sc->hide(o);
}

void evas_smart_preserve_window_smart_resize(Evas_Object* o,
                                             Evas_Coord w,
                                             Evas_Coord h) {
  Evas_Coord ow, oh;
  evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
  if ((ow == w) && (oh == h))
    return;

  PreserveWindowData* smart_data = ToSmartData(o);
  XResizeWindow(smart_data->xdisplay, smart_data->xwindow, w, h);
}

/* setting our smart interface */
void evas_smart_preserve_window_smart_set_user(Evas_Smart_Class* sc) {
  /* specializing these two */
  sc->add = evas_smart_preserve_window_smart_add;
  sc->del = evas_smart_preserve_window_smart_del;
  sc->show = evas_smart_preserve_window_smart_show;
  sc->hide = evas_smart_preserve_window_smart_hide;

  /* clipped smart object has no hook on resize */
  sc->resize = evas_smart_preserve_window_smart_resize;
}

}  // namespace

PreserveWindow::PreserveWindow(Evas* evas) {
  smart_object_ =
      evas_object_smart_add(evas, evas_smart_preserve_window_smart_class_new());
}

PreserveWindow::~PreserveWindow() {
  evas_object_del(smart_object_);
}

::Window PreserveWindow::GetXWindow() const {
  PreserveWindowData* smart_data = ToSmartData(smart_object_);
  return smart_data->xwindow;
}

}  // namespace views
