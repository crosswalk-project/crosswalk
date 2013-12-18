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

  set_name("xwalk.screen");
  set_entry_points(entry_points);
  set_javascript_api(ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SCREEN_ORIENTATION_API).as_string());
}

ScreenOrientationExtension::~ScreenOrientationExtension() {
}

XWalkExtensionInstance* ScreenOrientationExtension::CreateInstance() {
  RuntimeList runtimes = RuntimeRegistry::Get()->runtimes();
  DCHECK(!runtimes.empty());
  return new ScreenOrientationInstance(runtimes.back());
}

ScreenOrientationInstance::ScreenOrientationInstance(Runtime* runtime)
    : handler_(this) {
  // FIXME: Let NativeAppWindow inherit the interface.
  NativeAppWindowTizen* window
      = static_cast<NativeAppWindowTizen*>(runtime->window());
  supplement_ = static_cast<ScreenOrientationAPISupplement*>(window);

  handler_.Register("lock",
      base::Bind(&ScreenOrientationInstance::OnAllowedOrientationsChanged,
      base::Unretained(this)));

  // FIXME: Send UA supported orientations.
}

ScreenOrientationInstance::~ScreenOrientationInstance() {
}

void ScreenOrientationInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void ScreenOrientationInstance::OnAllowedOrientationsChanged(
    scoped_ptr<XWalkExtensionFunctionInfo> info) {
  int value;
  if (!info->arguments()->GetInteger(0, &value)) {
    LOG(WARNING) << "Malformed message passed to " << info->name();
    return;
  }

  OrientationMask orientations = static_cast<OrientationMask>(value);
  supplement_->OnAllowedOrientationsChanged(orientations);
}

}  // namespace xwalk
