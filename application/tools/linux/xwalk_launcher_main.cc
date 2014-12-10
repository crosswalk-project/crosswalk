// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <glib.h>

#include <memory>

#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/message_pump_glib.h"
#include "base/run_loop.h"

#include "xwalk/application/tools/linux/xwalk_launcher.h"
#if defined(OS_TIZEN)
#include "xwalk/application/tools/linux/xwalk_launcher_tizen.h"
#endif

namespace {

int g_argc;
char** g_argv;
gboolean query_running = FALSE;
gboolean fullscreen = FALSE;
gboolean remote_debugging = FALSE;
gchar** cmd_appid_or_url;
char* application_object_path;

}  // namespace

static const GOptionEntry entries[] {
  { "running", 'r', 0, G_OPTION_ARG_NONE, &query_running,
    "Check whether the application is running", nullptr },
  { "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &fullscreen,
    "Run the application as fullscreen", nullptr },
  { "debugging_port", 'd', 0, G_OPTION_ARG_NONE, &remote_debugging,
    "Enable remote debugging for the application", nullptr },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY,
    &cmd_appid_or_url,
    "ID of the application to be launched or URL to open", nullptr },
  { nullptr }
};

int main(int argc, char** argv) {
#if !GLIB_CHECK_VERSION(2, 36, 0)
  // g_type_init() is deprecated on GLib since 2.36.
  g_type_init();
#endif

#if defined(OS_TIZEN)
  if (xwalk_tizen_check_group_users())
    return 1;
#endif
  base::MessageLoop msg_loop(
      make_scoped_ptr<base::MessagePump>(new base::MessagePumpGlib()));

  g_argc = argc;
  g_argv = argv;

  GError* error = nullptr;
  GOptionContext* context =
      g_option_context_new("- Crosswalk Application Launcher");
  g_option_context_add_main_entries(context, entries, nullptr);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    LOG(ERROR) << "Option parsing failed: " << error->message;
    exit(1);
  }

  std::string appid_or_url;
  if (!strcmp(basename(g_argv[0]), "xwalk-launcher")) {
    if (!cmd_appid_or_url) {
      LOG(ERROR) << "No AppID informed, nothing to do.";
      exit(1);
    }
    appid_or_url = std::string(cmd_appid_or_url[0]);
  } else {
    appid_or_url = std::string(basename(g_argv[0]));
  }

  std::unique_ptr<XWalkLauncher> launcher;
#if defined(OS_TIZEN)
  launcher.reset(new XWalkLauncherTizen(query_running, &msg_loop));
#else
  launcher.reset(new XWalkLauncher(query_running, &msg_loop));
#endif
  int result = launcher->Launch(appid_or_url, fullscreen, remote_debugging,
                                argc, argv);
  if (!result)
    msg_loop.Run();
  return result;
}
