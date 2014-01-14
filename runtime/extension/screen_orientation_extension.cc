// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/extension/screen_orientation_extension.h"

#include <set>
#include <string>
#include <sstream>
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

namespace {

MultiOrientationScreen* GetMultiOrientationScreen(Application* app) {
  // FIXME(Mikhail): handle multi-windowed applications properly.
  // At the moment app has two runtimes: main (without window)
  // and a runtime with window.
  NativeAppWindowTizen* window = NULL;
  const std::set<Runtime*>& runtimes = app->runtimes();
  std::set<Runtime*>::const_iterator it = runtimes.begin();
  for (; it != runtimes.end(); ++it) {
    if (NativeAppWindow* native_window = (*it)->window()) {
      // FIXME: Let NativeAppWindow inherit the interface.
      window = static_cast<NativeAppWindowTizen*>(native_window);
      break;
    }
  }

  DCHECK(window);
  return static_cast<MultiOrientationScreen*>(window);
}

}  // namespace.

ScreenOrientationExtension::ScreenOrientationExtension(
    Application* app, OrientationMask ua_default)
  : application_(app) {
  DCHECK(application_);

  std::vector<std::string> entry_points;

  // FIXME: The on demand loading doesn't work:
  // Test case: http://jsbin.com/IJapIVE/6
  entry_points.push_back("screen");
  // entry_points.push_back("screen.lockOrientation");
  // entry_points.push_back("screen.unlockOrientation");
  // entry_points.push_back("screen.onorientationchange");

  set_name("xwalk.screen");
  set_entry_points(entry_points);

  std::ostringstream js_source;
  js_source << "var uaDefault = ";
  js_source << ua_default;
  js_source << ";\n";
  js_source << ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_XWALK_SCREEN_ORIENTATION_API).as_string();

  set_javascript_api(js_source.str());
}

ScreenOrientationExtension::~ScreenOrientationExtension() {
}

XWalkExtensionInstance* ScreenOrientationExtension::CreateInstance() {
  return new ScreenOrientationInstance(application_);
}

ScreenOrientationInstance::ScreenOrientationInstance(Application* app)
  : handler_(this)
  , screen_(GetMultiOrientationScreen(app))
  , application_(app) {
  screen_->SetObserver(this);

  handler_.Register("lock",
      base::Bind(&ScreenOrientationInstance::OnAllowedOrientationsChanged,
      base::Unretained(this)));

  // If the orientation property is not queried immediately after loading
  // we can avoid querying it synchronously.
  // FIXME: Make sure the .orientation property is 100% correct when queried.
  content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
    base::Bind(&ScreenOrientationInstance::OnOrientationChanged,
        base::Unretained(this), screen_->GetCurrentOrientation()));
}

ScreenOrientationInstance::~ScreenOrientationInstance() {
}

void ScreenOrientationInstance::OnOrientationChanged(Orientation orientation) {
  PostMessageToJS(scoped_ptr<base::Value>(
      base::Value::CreateIntegerValue(orientation)));
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
  screen_->OnAllowedOrientationsChanged(orientations);
}

}  // namespace xwalk
