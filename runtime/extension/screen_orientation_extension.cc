// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/extension/screen_orientation_extension.h"

#include "content/public/browser/browser_thread.h"
#include "grit/xwalk_resources.h"
#include "ui/aura/root_window.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/transform.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

#include "xwalk/extensions/common/xwalk_extension_server.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/browser/ui/native_app_window_views.h"

using content::BrowserThread;

namespace xwalk {

namespace {

gfx::Transform GetRotationTransform(const gfx::Size& size,
                                    gfx::Display::Rotation rotation) {
  gfx::Transform transform;
  switch (rotation) {
    case gfx::Display::ROTATE_0:
      break;
    case gfx::Display::ROTATE_90:
      transform.Translate(size.width() - 1, 0);
      transform.Rotate(90);
      break;
    case gfx::Display::ROTATE_180:
      transform.Translate(size.width() - 1, size.height() - 1);
      transform.Rotate(180);
      break;
    case gfx::Display::ROTATE_270:
      transform.Translate(0, size.height() - 1);
      transform.Rotate(270);
      break;
    default:
      NOTREACHED();
  }
  return transform;
}

ScreenOrientation ToScreenOrientation(gfx::Display::Rotation rotation) {
  switch (rotation) {
    case gfx::Display::ROTATE_0:
      return PORTRAIT_PRIMARY;
    case gfx::Display::ROTATE_90:
      return LANDSCAPE_PRIMARY;
    case gfx::Display::ROTATE_180:
      return PORTRAIT_SECONDARY;
    case gfx::Display::ROTATE_270:
      return  LANDSCAPE_SECONDARY;
  }
  NOTREACHED();
  return PORTRAIT_PRIMARY;
}

gfx::Display::Rotation ToDisplayRotation(ScreenOrientation orientation) {
  switch (orientation) {
    case PORTRAIT_PRIMARY:
      return gfx::Display::ROTATE_0;
    case PORTRAIT_SECONDARY:
      return gfx::Display::ROTATE_180;
    case LANDSCAPE_PRIMARY:
      return gfx::Display::ROTATE_90;
    case LANDSCAPE_SECONDARY:
      return gfx::Display::ROTATE_270;
    default:
      break;
  }
  NOTREACHED();
  return gfx::Display::ROTATE_0;
}

// This is a trick to ensure that ScreenOrientation extension
// must be loaded immediately.
class Preloader : public XWalkExtension {
 public:
  explicit Preloader() {
      set_name("xwalk.screen.preloader");
      set_javascript_api("//preload screen orientation extension");
  }
  virtual ~Preloader() {}

  virtual XWalkExtensionInstance* CreateInstance() OVERRIDE {
    return new DummyInstance();
  }

 private:
  class DummyInstance : public XWalkExtensionInstance {
   public:
    explicit DummyInstance() {}
    virtual ~DummyInstance() {}

    virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE {}
  };
};

}  // namespace

ScreenOrientationExtension::ScreenOrientationExtension(
    extensions::XWalkExtensionServer* server) {
  set_name("xwalk.screen");
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SCREEN_ORIENTATION_API).as_string());

  server->RegisterExtension(scoped_ptr<XWalkExtension>(new Preloader()));
}

ScreenOrientationExtension::~ScreenOrientationExtension() {
}

XWalkExtensionInstance* ScreenOrientationExtension::CreateInstance() {
  // FIXME(zliang7): Need a better way to map extension instance to runtime.
  RuntimeList runtimes = RuntimeRegistry::Get()->runtimes();
  DCHECK(!runtimes.empty());
  return new ScreenOrientationInstance(runtimes.back());
}

ScreenOrientationInstance::ScreenOrientationInstance(Runtime* runtime)
  : runtime_(runtime),
    handler_(this),
    allowed_orientations_(ANY_ORIENTATION),
    device_orientation_(PORTRAIT_PRIMARY),
    screen_orientation_(PORTRAIT_PRIMARY) {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->AddObserver(this);
  handler_.Register("lock",
      base::Bind(&ScreenOrientationInstance::OnLock, base::Unretained(this)));
}

ScreenOrientationInstance::~ScreenOrientationInstance() {
  if (SensorProvider::GetInstance())
    SensorProvider::GetInstance()->RemoveObserver(this);
}

void ScreenOrientationInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  CHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  handler_.HandleMessage(msg.Pass());
}

void ScreenOrientationInstance::OnLock(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  int value;
  if (!info->arguments()->GetInteger(0, &value)) {
    LOG(WARNING) << "Malformed message passed to " << info->name();
    return;
  }
  value = value? (value & ANY_ORIENTATION): ANY_ORIENTATION;
  allowed_orientations_ = static_cast<ScreenOrientation>(value);

  // Change to the real device orientation if it was disabled but allowed now.
  if ((device_orientation_ != screen_orientation_) &&
      (device_orientation_ & allowed_orientations_))
    OnRotationChanged(ToDisplayRotation(device_orientation_));

  // Change to a locked orientation, if the current orientation is disabled.
  if (!(screen_orientation_ & allowed_orientations_)) {
    for (value = 1; !(value & allowed_orientations_); value <<= 1) {
    }
    OnRotationChanged(ToDisplayRotation(static_cast<ScreenOrientation>(value)));
  }
}

void ScreenOrientationInstance::OnRotationChanged(
    gfx::Display::Rotation rotation) {
  // Do nothing if the orientation is not allowed or unchanged.
  device_orientation_ = ToScreenOrientation(rotation);
  if (device_orientation_ == screen_orientation_ ||
      !(allowed_orientations_ & device_orientation_))
    return;
  screen_orientation_ = device_orientation_;

  // Set rotation transform for root window. The size of the root window
  // will be changed automaticlly while the transform is set.
  NativeAppWindow* window = runtime_->window();
  aura::Window* native_window = window->GetNativeWindow();
  if (!native_window->GetRootWindow())
    return;
  native_window->GetRootWindow()->SetTransform(
      GetRotationTransform(window->GetBounds().size(), rotation));

  // Adjust the size of sub-windows.
  gfx::Rect bounds = window->GetBounds();
  if (rotation == gfx::Display::ROTATE_90 ||
      rotation == gfx::Display::ROTATE_270) {
    int width = bounds.width();
    bounds.set_width(bounds.height());
    bounds.set_height(width);
  }
  native_window->parent()->SetBounds(bounds);
  native_window->SetBounds(bounds);
  // FIXME(zliang7): change the size of content window without using widget.
  NativeAppWindowViews* views = static_cast<NativeAppWindowViews*>(window);
  views->GetWidget()->GetRootView()->SetSize(bounds.size());

  // Send current screen orientation value to JavaScript.
  PostMessageToJS(scoped_ptr<base::Value>(
      base::Value::CreateIntegerValue(screen_orientation_)));
}

}  // namespace xwalk
