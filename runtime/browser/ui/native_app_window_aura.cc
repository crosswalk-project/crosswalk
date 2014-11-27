// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_aura.h"

#include "base/command_line.h"
#include "content/public/browser/web_contents.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "ui/aura/client/aura_constants.h"
#include "ui/wm/core/default_activation_client.h"
#include "ui/aura/client/default_capture_client.h"
#include "ui/aura/env.h"
#include "ui/aura/layout_manager.h"
#include "ui/aura/test/test_focus_client.h"
#include "ui/aura/test/test_screen.h"
#include "ui/aura/test/test_window_tree_client.h"
#include "ui/aura/window.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/base/cursor/cursor.h"
#include "ui/base/cursor/image_cursors.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/input_method_delegate.h"
#include "ui/base/ime/input_method_factory.h"
#include "ui/gfx/screen.h"
#include "ui/wm/core/cursor_manager.h"
#include "ui/wm/core/native_cursor_manager.h"
#include "ui/wm/core/native_cursor_manager_delegate.h"

namespace xwalk {

const int kDefaultTestWindowWidthDip = 800;
const int kDefaultTestWindowHeightDip = 600;

const char kXwalkHostWindowBounds[] = "xwalk-host-window-bounds";

namespace {

class FillLayout : public aura::LayoutManager {
 public:
  explicit FillLayout(aura::Window* root)
      : root_(root) {
  }

  virtual ~FillLayout() {}

 private:
  // aura::LayoutManager:
  virtual void OnWindowResized() override {
  }

  virtual void OnWindowAddedToLayout(aura::Window* child) override {
    child->SetBounds(root_->bounds());
  }

  virtual void OnWillRemoveWindowFromLayout(aura::Window* child) override {
  }

  virtual void OnWindowRemovedFromLayout(aura::Window* child) override {
  }

  virtual void OnChildWindowVisibilityChanged(aura::Window* child,
                                              bool visible) override {
  }

  virtual void SetChildBounds(aura::Window* child,
                              const gfx::Rect& requested_bounds) override {
    SetChildBoundsDirect(child, requested_bounds);
  }

  aura::Window* root_;

  DISALLOW_COPY_AND_ASSIGN(FillLayout);
};

// Taken from app_shell.
// A class that bridges the gap between CursorManager and Aura. It borrows
// heavily from AshNativeCursorManager.
class ShellNativeCursorManager : public wm::NativeCursorManager {
 public:
  explicit ShellNativeCursorManager(aura::WindowTreeHost* host)
      : host_(host),
        image_cursors_(new ui::ImageCursors) {}
  virtual ~ShellNativeCursorManager() {}

  // wm::NativeCursorManager overrides.
  virtual void SetDisplay(
      const gfx::Display& display,
      wm::NativeCursorManagerDelegate* delegate) override {
    if (image_cursors_->SetDisplay(display, display.device_scale_factor()))
      SetCursor(delegate->GetCursor(), delegate);
  }

  virtual void SetCursor(
      gfx::NativeCursor cursor,
      wm::NativeCursorManagerDelegate* delegate) override {
    image_cursors_->SetPlatformCursor(&cursor);
    cursor.set_device_scale_factor(image_cursors_->GetScale());
    delegate->CommitCursor(cursor);

    if (delegate->IsCursorVisible())
      ApplyCursor(cursor);
  }

  virtual void SetVisibility(
      bool visible,
      wm::NativeCursorManagerDelegate* delegate) override {
    delegate->CommitVisibility(visible);

    if (visible) {
      SetCursor(delegate->GetCursor(), delegate);
    } else {
      gfx::NativeCursor invisible_cursor(ui::kCursorNone);
      image_cursors_->SetPlatformCursor(&invisible_cursor);
      ApplyCursor(invisible_cursor);
    }
  }

  virtual void SetCursorSet(
      ui::CursorSetType cursor_set,
      wm::NativeCursorManagerDelegate* delegate) override {
    image_cursors_->SetCursorSet(cursor_set);
    delegate->CommitCursorSet(cursor_set);
    if (delegate->IsCursorVisible())
      SetCursor(delegate->GetCursor(), delegate);
  }

  virtual void SetMouseEventsEnabled(
      bool enabled,
      wm::NativeCursorManagerDelegate* delegate) override {
    delegate->CommitMouseEventsEnabled(enabled);
    SetVisibility(delegate->IsCursorVisible(), delegate);
  }

 private:
  // Sets |cursor| as the active cursor within Aura.
  void ApplyCursor(gfx::NativeCursor cursor) {
    host_->SetCursor(cursor);
  }

  aura::WindowTreeHost* host_;  // Not owned.

  scoped_ptr<ui::ImageCursors> image_cursors_;

  DISALLOW_COPY_AND_ASSIGN(ShellNativeCursorManager);
};

class MinimalInputEventFilter : public ui::internal::InputMethodDelegate,
                                public ui::EventHandler {
 public:
  explicit MinimalInputEventFilter(aura::WindowTreeHost* host)
      : host_(host),
        input_method_(ui::CreateInputMethod(this,
                                            gfx::kNullAcceleratedWidget)) {
    input_method_->Init(true);
    host_->window()->AddPreTargetHandler(this);
    host_->window()->SetProperty(aura::client::kRootWindowInputMethodKey,
                                 input_method_.get());
  }

  virtual ~MinimalInputEventFilter() {
    host_->window()->RemovePreTargetHandler(this);
    host_->window()->SetProperty(aura::client::kRootWindowInputMethodKey,
                                 static_cast<ui::InputMethod*>(NULL));
  }

 private:
  // ui::EventHandler:
  virtual void OnKeyEvent(ui::KeyEvent* event) override {
    // See the comment in InputMethodEventFilter::OnKeyEvent() for details.
    if (event->IsTranslated()) {
      event->SetTranslated(false);
    } else if (input_method_->DispatchKeyEvent(*event)) {
      event->StopPropagation();
    }
  }

  // ui::internal::InputMethodDelegate:
  virtual bool DispatchKeyEventPostIME(const ui::KeyEvent& event) override {
    // See the comment in InputMethodEventFilter::DispatchKeyEventPostIME() for
    // details.
    ui::KeyEvent aura_event(event);
    aura_event.SetTranslated(true);
    ui::EventDispatchDetails details =
        host_->dispatcher()->OnEventFromSource(&aura_event);
    return aura_event.handled() || details.dispatcher_destroyed;
  }

  aura::WindowTreeHost* host_;
  scoped_ptr<ui::InputMethod> input_method_;

  DISALLOW_COPY_AND_ASSIGN(MinimalInputEventFilter);
};

}  // namespace

// mimicking Shell::ShellPlatformDataAura
NativeAppWindowAura::NativeAppWindowAura(
    const NativeAppWindow::CreateParams& create_params)
    : web_contents_(create_params.web_contents) {
  aura::Env::CreateInstance(true);
  gfx::Size size;

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kXwalkHostWindowBounds)) {
    const std::string size_str =
        command_line->GetSwitchValueASCII(kXwalkHostWindowBounds);
    int width, height;
    if (sscanf(size_str.c_str(), "%d,%d", &width, &height) == 2)
      size = gfx::Size(width, height);
  }

  if (size.IsEmpty())
    size = gfx::Size(kDefaultTestWindowWidthDip, kDefaultTestWindowHeightDip);

  host_.reset(aura::WindowTreeHost::Create(gfx::Rect(size)));
  host_->InitHost();
  host_->window()->SetLayoutManager(new FillLayout(host_->window()));

  focus_client_.reset(new aura::test::TestFocusClient());
  aura::client::SetFocusClient(host_->window(), focus_client_.get());

  new wm::DefaultActivationClient(host_->window());
  capture_client_.reset(
      new aura::client::DefaultCaptureClient(host_->window()));
  window_tree_client_.reset(
      new aura::test::TestWindowTreeClient(host_->window()));
  ime_filter_.reset(new MinimalInputEventFilter(host_.get()));

  cursor_manager_.reset(
      new wm::CursorManager(scoped_ptr<wm::NativeCursorManager>(
          new ShellNativeCursorManager(host_.get()))));
  cursor_manager_->SetDisplay(
      gfx::Screen::GetNativeScreen()->GetPrimaryDisplay());
  cursor_manager_->SetCursor(ui::kCursorPointer);
  aura::client::SetCursorClient(host_->window(), cursor_manager_.get());

  // mimicking Shell::PlatformSetContents
  aura::Window* content = web_contents_->GetNativeView();
  aura::Window* parent = host_->window();
  if (!parent->Contains(content))
    parent->AddChild(content);

  content->Show();
  parent->Show();

  web_contents_->Focus();
}

NativeAppWindowAura::~NativeAppWindowAura() {
  aura::Env::DeleteInstance();
}

gfx::NativeWindow NativeAppWindowAura::GetNativeWindow() const {
  return NULL;
}

void NativeAppWindowAura::UpdateIcon(const gfx::Image& icon) {
}

void NativeAppWindowAura::UpdateTitle(const base::string16& title) {
}

gfx::Rect NativeAppWindowAura::GetRestoredBounds() const {
  return gfx::Rect();
}

gfx::Rect NativeAppWindowAura::GetBounds() const {
  return host_->GetBounds();
}

void NativeAppWindowAura::SetBounds(const gfx::Rect& bounds) {
  host_->SetBounds(bounds);
}

void NativeAppWindowAura::Focus() {
}

void NativeAppWindowAura::Show() {
  host_->Show();
}

void NativeAppWindowAura::Hide() {
  host_->Hide();
}

void NativeAppWindowAura::Maximize() {
}

void NativeAppWindowAura::Minimize() {
}

void NativeAppWindowAura::SetFullscreen(bool fullscreen) {
}

void NativeAppWindowAura::Restore() {
}

void NativeAppWindowAura::FlashFrame(bool flash) {
}

void NativeAppWindowAura::Close() {
}

bool NativeAppWindowAura::IsActive() const {
  return true;
}

bool NativeAppWindowAura::IsMaximized() const {
  return true;
}

bool NativeAppWindowAura::IsMinimized() const {
  return false;
}

bool NativeAppWindowAura::IsFullscreen() const {
  return true;
}

// static
NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& create_params) {
  return new NativeAppWindowAura(create_params);
}

// static
void NativeAppWindow::Initialize() {
  // At least Athena, Content Shell and Chromecast use the "TestScreen" class
  // not exactly for testing but production instead. So I think we are fine on
  // using it as long they are using it as well.
  aura::TestScreen* screen = aura::TestScreen::Create(gfx::Size());
  gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, screen);
}

}  // namespace xwalk
