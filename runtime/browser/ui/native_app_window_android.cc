// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_android.h"

#include "content/public/browser/web_contents.h"
#include "ui/gfx/rect.h"

namespace xwalk {

NativeAppWindowAndroid::NativeAppWindowAndroid(
    const NativeAppWindow::CreateParams& params)
    : delegate_(params.delegate),
      web_contents_(params.web_contents) {

}

NativeAppWindowAndroid::~NativeAppWindowAndroid() {
}

gfx::NativeWindow NativeAppWindowAndroid::GetNativeWindow() const {
    return 0;
}

void NativeAppWindowAndroid::UpdateIcon(const gfx::Image& icon) {
}

void NativeAppWindowAndroid::UpdateTitle(const string16& title) {
}

gfx::Rect NativeAppWindowAndroid::GetRestoredBounds() const {
  return GetBounds();
}

gfx::Rect NativeAppWindowAndroid::GetBounds() const {
  return gfx::Rect(0, 0, 0, 0);
}

void NativeAppWindowAndroid::SetBounds(const gfx::Rect& bounds) {
}

void NativeAppWindowAndroid::Focus() {
}

void NativeAppWindowAndroid::Show() {
}

void NativeAppWindowAndroid::Hide() {
}

void NativeAppWindowAndroid::Maximize() {
}

void NativeAppWindowAndroid::Minimize() {
}

void NativeAppWindowAndroid::SetFullscreen(bool fullscreen) {
}

void NativeAppWindowAndroid::Restore() {
}

void NativeAppWindowAndroid::FlashFrame(bool flash) {
}

void NativeAppWindowAndroid::Close() {
}

bool NativeAppWindowAndroid::IsActive() const {
    return true;
}

bool NativeAppWindowAndroid::IsMaximized() const {
  return true;
}

bool NativeAppWindowAndroid::IsMinimized() const {
  return false;
}

bool NativeAppWindowAndroid::IsFullscreen() const {
  return true;
}

NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& params) {
  return new NativeAppWindowAndroid(params);
}

void NativeAppWindow::Initialize() {
}

}  // namespace xwalk
