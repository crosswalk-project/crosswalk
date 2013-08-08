// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/tizen/xwindow_provider_efl.h"

#include <Ecore_X.h>
#include <Elementary.h>
#include "xwalk/runtime/browser/ui/tizen/preserve_window_efl.h"

namespace views {

namespace {

void Close(void* data, Evas_Object*, void*) {
  static_cast<XWindowProviderDelegate*>(data)->CloseWindow();
}

}

struct XWindowProvider::Private {
  XWindowProviderDelegate* delegate;
  Evas_Object* elm_window;
  Evas_Object* container;
  scoped_ptr<PreserveWindow> preserve_window;
  gfx::Rect bounds;
};

XWindowProvider::XWindowProvider(XWindowProviderDelegate* delegate,
                                 gfx::Rect bounds)
    : private_(new Private) {
  private_->delegate = delegate;
  private_->elm_window = elm_win_add(NULL, "XWalkWindow", ELM_WIN_BASIC);
  evas_object_smart_callback_add(private_->elm_window, "delete,request",
                                 Close, private_->delegate);
  // Delete the elementary window in DesktopRootWindowHostTizen::CloseNow().
  elm_win_autodel_set(private_->elm_window, EINA_FALSE);

  Resize(bounds.width(), bounds.height());
  Move(bounds.x(), bounds.y());

  private_->container = elm_box_add(private_->elm_window);
  elm_box_padding_set(private_->container, 0, 2);
  evas_object_size_hint_weight_set(private_->container,
                                   EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(private_->elm_window, private_->container);
  evas_object_show(private_->container);

  private_->preserve_window.reset(
      new PreserveWindow(evas_object_evas_get(private_->elm_window)));
  Evas_Object* xwindow_holder = private_->preserve_window->SmartObject();
  evas_object_size_hint_weight_set(xwindow_holder,
                                   EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(xwindow_holder,
                                  EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_pack_end(private_->container, xwindow_holder);
  evas_object_show(xwindow_holder);

  evas_object_show(private_->elm_window);
}

XWindowProvider::~XWindowProvider() {
  private_->preserve_window.reset();
  evas_object_del(private_->container);
  evas_object_del(private_->elm_window);
}

::Window XWindowProvider::GetXWindow() const {
  return private_->preserve_window->GetXWindow();
}

gfx::Rect XWindowProvider::GetBounds() const {
  return private_->bounds;
}

void XWindowProvider::Move(int x, int y) {
  private_->bounds.set_origin(gfx::Point(x, y));
  evas_object_move(private_->elm_window, x, y);
}

void XWindowProvider::Resize(int w, int h) {
  private_->bounds.set_size(gfx::Size(w, h));
  evas_object_resize(private_->elm_window, w, h);
}

void XWindowProvider::Show() {
  evas_object_show(private_->elm_window);
}

void XWindowProvider::Hide() {
  evas_object_hide(private_->elm_window);
}

// static
gfx::Rect XWindowProvider::GetWindowGeometry() {
  int x, y, width, height;
  ecore_x_window_geometry_get(ecore_x_window_root_first_get(),
                              &x, &y, &width, &height);
  return gfx::Rect(x, y, width, height);
}

}  // namespace views
