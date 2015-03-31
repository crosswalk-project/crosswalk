// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/x11_input_method_context_factory.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/X.h>

#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/debug/leak_annotations.h"
#include "ui/gfx/x/x11_types.h"
#include "xwalk/runtime/browser/ui/x11_input_method_context_impl_gtk2.h"

namespace {

void GtkInitFromCommandLine(const base::CommandLine& command_line) {
  const std::vector<std::string>& args = command_line.argv();
  int argc = args.size();
  scoped_ptr<char *[]> argv(new char *[argc + 1]);
  for (size_t i = 0; i < args.size(); ++i) {
    // TODO(piman@google.com): can gtk_init modify argv? Just being safe
    // here.
    argv[i] = strdup(args[i].c_str());
  }
  argv[argc] = NULL;
  char **argv_pointer = argv.get();

  {
    // http://crbug.com/423873
    ANNOTATE_SCOPED_MEMORY_LEAK;
    gtk_init(&argc, &argv_pointer);
  }
  for (size_t i = 0; i < args.size(); ++i) {
    free(argv[i]);
  }
}

void ProcessGdkEventKey(const GdkEventKey& gdk_event_key) {
  // This function translates GdkEventKeys into XKeyEvents and puts them to
  // the X event queue.
  //
  // base::MessagePumpX11 is using the X11 event queue and all key events should
  // be processed there.  However, there are cases(*1) that GdkEventKeys are
  // created instead of XKeyEvents.  In these cases, we have to translate
  // GdkEventKeys to XKeyEvents and puts them to the X event queue so our main
  // event loop can handle those key events.
  //
  // (*1) At least ibus-gtk in async mode creates a copy of user's key event and
  // pushes it back to the GDK event queue.  In this case, there is no
  // corresponding key event in the X event queue.  So we have to handle this
  // case.  ibus-gtk is used through gtk-immodule to support IMEs.

  XEvent x_event = {0};
  x_event.xkey.type =
      gdk_event_key.type == GDK_KEY_PRESS ? KeyPress : KeyRelease;
  x_event.xkey.send_event = gdk_event_key.send_event;
  x_event.xkey.display = gfx::GetXDisplay();
  x_event.xkey.window = GDK_WINDOW_XID(gdk_event_key.window);
  x_event.xkey.root = DefaultRootWindow(x_event.xkey.display);
  x_event.xkey.time = gdk_event_key.time;
  x_event.xkey.state = gdk_event_key.state;
  x_event.xkey.keycode = gdk_event_key.hardware_keycode;
  x_event.xkey.same_screen = true;

  XPutBackEvent(x_event.xkey.display, &x_event);
}

void DispatchGdkEvent(GdkEvent* gdk_event, gpointer) {
  switch (gdk_event->type) {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      ProcessGdkEventKey(gdk_event->key);
      break;
    default:
      break;  // Do nothing.
  }

  gtk_main_do_event(gdk_event);
}

}  // namespace

namespace xwalk {

X11InputMethodContextFactory::X11InputMethodContextFactory() {
  GtkInitFromCommandLine(*base::CommandLine::ForCurrentProcess());
  gdk_event_handler_set(DispatchGdkEvent, NULL, NULL);
}

X11InputMethodContextFactory::~X11InputMethodContextFactory() {
  gdk_event_handler_set(reinterpret_cast<GdkEventFunc>(gtk_main_do_event),
                        NULL, NULL);
}

scoped_ptr<ui::LinuxInputMethodContext>
X11InputMethodContextFactory::CreateInputMethodContext(
    ui::LinuxInputMethodContextDelegate* delegate) const {
  return make_scoped_ptr(new X11InputMethodContextImplGtk2(delegate));
}

}  // namespace xwalk
