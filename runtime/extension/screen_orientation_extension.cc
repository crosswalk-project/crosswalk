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
#include "third_party/jsoncpp/source/include/json/json.h"

#include "xwalk/application/browser/application.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/browser/ui/native_app_window_tizen.h"

using content::BrowserThread;

namespace xwalk {

using application::Application;

namespace {

MultiOrientationScreen* GetMultiOrientationScreen(Application*) {  // NOLINT
  NOTREACHED();
  return NULL;
}

}  // namespace.

ScreenOrientationExtension::ScreenOrientationExtension(
    Application* app, OrientationMask ua_default)
  : application_(app) {
  DCHECK(application_);

  std::vector<std::string> entry_points;
  entry_points.push_back("screen.orientation");
  entry_points.push_back("screen.lockOrientation");
  entry_points.push_back("screen.unlockOrientation");
  entry_points.push_back("screen.onorientationchange");

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

void ScreenOrientationInstance::HandleMessage(
    scoped_ptr<base::Value> msg) {
  handler_.HandleMessage(msg.Pass());
}

void ScreenOrientationInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  std::string message;
  msg->GetAsString(&message);

  Json::Value input;
  Json::Reader reader;
  if (!reader.parse(message, input))
    LOG(ERROR) << "Failed to parse message: " << message;
  else if (input["cmd"].asString() != "GetScreenOrientation")
    LOG(ERROR) << "Got unknown command: " << input["cmd"].asString();

  SendSyncReplyToJS(scoped_ptr<base::Value>(
      base::Value::CreateIntegerValue(screen_->GetCurrentOrientation())));
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
