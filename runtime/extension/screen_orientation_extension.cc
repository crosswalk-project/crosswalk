// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/extension/screen_orientation_extension.h"

#include <set>
#include <string>
#include <vector>

#include "content/public/browser/browser_thread.h"
#include "grit/xwalk_resources.h"
#include "ui/base/resource/resource_bundle.h"

#include "xwalk/application/browser/application.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

using content::BrowserThread;

namespace xwalk {

using application::Application;

ScreenOrientationExtension::ScreenOrientationExtension(Application* app)
  : application_(app) {
  DCHECK(application_);
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
  return new ScreenOrientationInstance(application_);
}

ScreenOrientationInstance::ScreenOrientationInstance(Application* app)
  : handler_(this),
  application_(app) {
  // FIXME(Mikhail): handle multi-windowed applications properly.
  // At the moment app has two runtimes: main (without window)
  // and a runtime with window.
  NativeAppWindowTizen* window = NULL;
  const std::set<Runtime*>& runtimes = application_->runtimes();
  std::set<Runtime*>::const_iterator it = runtimes.begin();
  for (; it != runtimes.end(); ++it) {
    if (NativeAppWindow* native_window = (*it)->window()) {
      // FIXME: Let NativeAppWindow inherit the interface.
      window = static_cast<NativeAppWindowTizen*>(native_window);
      break;
    }
  }
  DCHECK(window);
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
