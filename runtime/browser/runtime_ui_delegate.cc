// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_ui_delegate.h"

#include "base/command_line.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image.h"
#include "xwalk/runtime/browser/image_util.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {
// FIXME : Need to figure out what code paths are used by Android and not
// compile the unneeded files.
#if !defined(OS_ANDROID)
namespace {
// The default size for web content area size.
const int kDefaultWidth = 840;
const int kDefaultHeight = 600;

NativeAppWindow* RuntimeCreateWindow(
    Runtime* runtime, const NativeAppWindow::CreateParams& params) {
  NativeAppWindow* window = NativeAppWindow::Create(params);
  // FIXME : Pass an App icon in params.
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

  window->UpdateIcon(app_icon);

  unsigned int fullscreen_options = runtime->fullscreen_options();
  if (params.state == ui::SHOW_STATE_FULLSCREEN)
    fullscreen_options |= Runtime::FULLSCREEN_FOR_LAUNCH;
  else
    fullscreen_options &= ~Runtime::FULLSCREEN_FOR_LAUNCH;
  runtime->set_fullscreen_options(fullscreen_options);

  return window;
}

}  // namespace
#endif

RuntimeUIDelegate* DefaultRuntimeUIDelegate::Create(
    Runtime* runtime, const NativeAppWindow::CreateParams& params) {
  return new DefaultRuntimeUIDelegate(runtime, params);
}

DefaultRuntimeUIDelegate::DefaultRuntimeUIDelegate(
    Runtime* runtime, const NativeAppWindow::CreateParams& params)
  : runtime_(runtime),
    window_params_(params),
    window_(nullptr) {
  DCHECK(runtime_);
}

DefaultRuntimeUIDelegate::~DefaultRuntimeUIDelegate() {
}

void DefaultRuntimeUIDelegate::Show() {
#if !defined(OS_ANDROID)
  if (!window_) {
    if (window_params_.bounds.IsEmpty())
      window_params_.bounds = gfx::Rect(0, 0, kDefaultWidth, kDefaultHeight);
    window_params_.delegate = this;
    window_params_.web_contents = runtime_->web_contents();
    window_ = RuntimeCreateWindow(runtime_, window_params_);
  }
  window_->Show();
#else
  NOTIMPLEMENTED();
#endif
}

void DefaultRuntimeUIDelegate::UpdateTitle(const base::string16& text) {
  if (window_)
    window_->UpdateTitle(text);
}

void DefaultRuntimeUIDelegate::UpdateIcon(const gfx::Image& image) {
  if (window_)
    window_->UpdateIcon(image);
}

void DefaultRuntimeUIDelegate::SetFullscreen(bool enter_fullscreen) {
  if (window_)
    window_->SetFullscreen(enter_fullscreen);
}

void DefaultRuntimeUIDelegate::Close() {
  if (window_)
    window_->Close();
}

void DefaultRuntimeUIDelegate::DeleteDelegate() {
  runtime_ = nullptr;
  if (window_) {
    window_->Close();
    return;
  }
  delete this;
}

void DefaultRuntimeUIDelegate::OnWindowDestroyed() {
  window_ = nullptr;
  if (runtime_) {
    runtime_->Close();
    return;
  }
  delete this;
}

}  // namespace xwalk
