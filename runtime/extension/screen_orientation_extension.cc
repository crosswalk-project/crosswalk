// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/extension/screen_orientation_extension.h"

#include <string>
#include <vector>

#include "content/public/browser/browser_thread.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"

#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/runtime_registry.h"
#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

using content::BrowserThread;

namespace xwalk {

ScreenOrientationExtension::ScreenOrientationExtension() {
  std::vector<std::string> entry_points;
  entry_points.push_back("screen.lockOrientation");
  entry_points.push_back("screen.unlockOrientation");
  entry_points.push_back("screen.orientation");
  entry_points.push_back("screen.addEventListener");
  entry_points.push_back("screen.removeEventListener");
  entry_points.push_back("screen.dispatchEvent");
  entry_points.push_back("screen.onorientationchange");

  set_name("xwalk.screen");
  set_entry_points(entry_points);
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SCREEN_ORIENTATION_API).as_string());
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
  : handler_(this) {
  NativeAppWindow* window = runtime->window();
  window_ = static_cast<NativeAppWindowTizen*>(window);
  window_->AddObserver(this);

  handler_.Register("lock",
      base::Bind(&ScreenOrientationInstance::OnLock, base::Unretained(this)));

  // Send initial orientation to javascript
  content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ScreenOrientationInstance::OnScreenOrientationChanged,
                 base::Unretained(this),
                 window_->GetOrientation()));
}

ScreenOrientationInstance::~ScreenOrientationInstance() {
  window_->RemoveObserver(this);
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
  value = value? value & ScreenOrientation::ANY_ORIENTATION
               : ScreenOrientation::ANY_ORIENTATION;
  window_->LockOrientation(static_cast<ScreenOrientation::Mode>(value));
}

void ScreenOrientationInstance::OnScreenOrientationChanged(
    ScreenOrientation::Mode mode) {
  // Send current screen orientation value to JavaScript.
  PostMessageToJS(scoped_ptr<base::Value>(
      base::Value::CreateIntegerValue(mode)));
}

}  // namespace xwalk
