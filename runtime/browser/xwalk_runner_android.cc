// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner_android.h"

#include "xwalk/runtime/browser/application_component.h"
#include "xwalk/runtime/browser/sysapps_component.h"

namespace xwalk {

XWalkRunnerAndroid::XWalkRunnerAndroid() {}

XWalkRunnerAndroid::~XWalkRunnerAndroid() {}

// static
XWalkRunnerAndroid* XWalkRunnerAndroid::GetInstance() {
  return static_cast<XWalkRunnerAndroid*>(XWalkRunner::GetInstance());
}

scoped_ptr<ApplicationComponent> XWalkRunnerAndroid::CreateAppComponent() {
  scoped_ptr<ApplicationComponent> app_component(
      XWalkRunner::CreateAppComponent());
  // FIXME(cmarcelo): The application extensions are currently not fully working
  // for Android, so disable them. See
  // https://crosswalk-project.org/jira/browse/XWALK-674.
  app_component->DisableExtensions();
  return app_component.Pass();
}

scoped_ptr<SysAppsComponent> XWalkRunnerAndroid::CreateSysAppsComponent() {
  scoped_ptr<SysAppsComponent> sysapps_component(
      XWalkRunner::CreateSysAppsComponent());
  // Android uses a Java extension for device capabilities, so disable the one
  // from sysapps/.
  sysapps_component->DisableDeviceCapabilities();
  return sysapps_component.Pass();
}

}  // namespace xwalk
