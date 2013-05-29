// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_UI_GTK_GTK_UTIL_H_
#define CAMEO_SRC_RUNTIME_BROWSER_UI_GTK_GTK_UTIL_H_

#include <gtk/gtk.h>
#include <string>
#include <vector>

#include "base/string16.h"
#include "ui/base/window_open_disposition.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"

typedef struct _GtkWidget GtkWidget;

namespace gtk_util {

// As above, but uses number of characters/lines directly rather than looking up
// a resource.
void GetWidgetSizeFromCharacters(GtkWidget* widget,
                                 double width_chars, double height_lines,
                                 int* width, int* height);

// Calculates the size of given widget based on the size specified in number of
// characters/lines (in locale specific resource file) and font metrics.
// NOTE: Make sure to realize |widget| before using this method, or a default
// font size will be used instead of the actual font size.
void GetWidgetSizeFromResources(GtkWidget* widget,
                                int width_chars, int height_lines,
                                int* width, int* height);

void ShowDialogWithMinLocalizedWidth(GtkWidget* dialog,
                                     int width_id);

// A helper function for gtk_message_dialog_new() to work around a few KDE 3
// window manager bugs. You should always call it after creating a dialog with
// gtk_message_dialog_new.
void ApplyMessageDialogQuirks(GtkWidget* dialog);

}  // namespace gtk_util

#endif  // CAMEO_SRC_RUNTIME_BROWSER_UI_GTK_GTK_UTIL_H_
