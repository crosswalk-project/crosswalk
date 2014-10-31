// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_ui_strategy.h"

#include "base/command_line.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {
namespace {
#if !defined(OS_ANDROID)
// The default size for web content area size.
const int kDefaultWidth = 840;
const int kDefaultHeight = 600;

void ApplyWindowDefaultParams(Runtime* runtime,
                              NativeAppWindow::CreateParams* params) {
  if (!params->delegate)
    params->delegate = runtime;
  if (!params->web_contents)
    params->web_contents = runtime->web_contents();
  if (params->bounds.IsEmpty())
    params->bounds = gfx::Rect(0, 0, kDefaultWidth, kDefaultHeight);

  unsigned int fullscreen_options = runtime->fullscreen_options();
  if (params->state == ui::SHOW_STATE_FULLSCREEN)
    fullscreen_options |= Runtime::FULLSCREEN_FOR_LAUNCH;
  else
    fullscreen_options &= ~Runtime::FULLSCREEN_FOR_LAUNCH;
  runtime->set_fullscreen_options(fullscreen_options);
}
#endif
}  // namespace

RuntimeUIStrategy::RuntimeUIStrategy() {}

RuntimeUIStrategy::~RuntimeUIStrategy() {}

void RuntimeUIStrategy::Show(
    Runtime* runtime, const NativeAppWindow::CreateParams& params) {
#if defined(OS_ANDROID)
  NOTIMPLEMENTED();
#else
  NativeAppWindow::CreateParams effective_params(params);
  ApplyWindowDefaultParams(runtime, &effective_params);

  // Set the app icon if it is passed from command line.
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  gfx::Image app_icon;
  if (command_line->HasSwitch(switches::kAppIcon)) {
    base::FilePath icon_file =
        command_line->GetSwitchValuePath(switches::kAppIcon);
    app_icon = xwalk_utils::LoadImageFromFilePath(icon_file);
  } else {
    // Otherwise, use the default icon for Crosswalk app.
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
    app_icon = rb.GetNativeImageNamed(IDR_XWALK_ICON_48);
  }

  runtime->EnableTitleUpdatedNotification();

  NativeAppWindow* window = NativeAppWindow::Create(effective_params);
  runtime->set_window(window);

  runtime->set_app_icon(app_icon);
  window->Show();
#endif
}
}  // namespace xwalk
