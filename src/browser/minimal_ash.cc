// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/minimal_ash.h"

#include "ui/aura/client/default_capture_client.h"
#include "ui/aura/focus_manager.h"
#include "ui/aura/root_window.h"
#include "ui/aura/test/test_activation_client.h"
#include "ui/views/corewm/compound_event_filter.h"
#include "ui/views/corewm/input_method_event_filter.h"

namespace content {

MinimalAsh::MinimalAsh(const gfx::Size& default_window_size) {
  root_window_.reset(new aura::RootWindow(
      aura::RootWindow::CreateParams(
          gfx::Rect(default_window_size))));
  root_window_->Init();
  aura::client::SetStackingClient(root_window_.get(), this);

  focus_client_.reset(new aura::FocusManager);
  aura::client::SetFocusClient(root_window_.get(), focus_client_.get());

  root_window_event_filter_ = new views::corewm::CompoundEventFilter;
  // Pass ownership of the filter to the root_window.
  root_window_->SetEventFilter(root_window_event_filter_);

  input_method_filter_.reset(new views::corewm::InputMethodEventFilter(
      root_window_->GetAcceleratedWidget()));
  input_method_filter_->SetInputMethodPropertyInRootWindow(
      root_window_.get());
  root_window_event_filter_->AddHandler(input_method_filter_.get());

  test_activation_client_.reset(
      new aura::test::TestActivationClient(root_window_.get()));

  capture_client_.reset(
      new aura::client::DefaultCaptureClient(root_window_.get()));
}

MinimalAsh::~MinimalAsh() {
  root_window_event_filter_->RemoveHandler(input_method_filter_.get());
}

aura::Window* MinimalAsh::GetDefaultParent(
    aura::Window* context,
    aura::Window* window,
    const gfx::Rect& bounds) {
  return root_window_.get();
}

}  // namespace content
