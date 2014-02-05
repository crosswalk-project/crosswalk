// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DIALOG_LAUNCHER_H
#define DIALOG_LAUNCHER_H

typedef struct OpenedDialog OpenedDialog;
typedef enum DialogType DialogType;
typedef struct DialogLauncher DialogLauncher;

enum DialogType {
  DIALOG_TYPE_DATE_TIME = 1,
  DIALOG_TYPE_FILE_PICKER,
  DIALOG_TYPE_COLOR_CHOOSER,
  DIALOG_TYPE_SELECTION
};

struct OpenedDialog {
  int uid;
  DialogType type;
  void* dialog;
  Evas_Object* window;
};

extern const char kDialogLauncherServiceName[];
extern const char kDialogLauncherInterfaceName[];
extern const char kDialogLauncherObjectPath[];

DialogLauncher* create_service();
Eina_Bool register_service(DialogLauncher* service);
void shutdown_service(DialogLauncher* service);

#endif // DIALOG_LAUNCHER
