// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_runtime_features.h"

#include <algorithm>
#include <functional>

#include "base/message_loop/message_loop.h"

namespace xwalk {

XWalkRuntimeFeatures* g_features = NULL;

struct MatchRuntimeFeature
  : std::unary_function<XWalkRuntimeFeatures::RuntimeFeature, bool> {
  explicit MatchRuntimeFeature(const std::string& name) : name(name) {}
  bool operator()(const XWalkRuntimeFeatures::RuntimeFeature& entry) const {
    return entry.name == name;
  }
  const std::string name;
};

// static
void XWalkRuntimeFeatures::Initialize(const CommandLine* cmd) {
  g_features = new XWalkRuntimeFeatures(cmd);
}

// static
XWalkRuntimeFeatures* XWalkRuntimeFeatures::GetInstance() {
  return g_features;
}

XWalkRuntimeFeatures::XWalkRuntimeFeatures(const CommandLine* cmd)
  : command_line_(cmd) {
  // Add new features here with the following parameters :
  // - Name of the feature
  // - Name of the command line switch which will be used after the
  // --enable/--disable
  // - Description of the feature
  // - Status of the feature : experimental which is turned off by default or
  // stable which is turned on by default
  AddFeature("RawSocketsAPI", "raw-sockets",
             "JavaScript support for using TCP and UDP sockets", Stable);
  AddFeature("DeviceCapabilitiesAPI", "device-capabilities",
             "JavaScript support for peeking at device capabilities", Stable);
}

XWalkRuntimeFeatures::~XWalkRuntimeFeatures() {}

void XWalkRuntimeFeatures::AddFeature(const char* name,
                                 const char* command_line_switch,
                                 const char* description,
                                 RuntimeFeatureStatus status) {
  RuntimeFeature feature;
  feature.name = name;

  if (command_line_->HasSwitch(
              ("disable-" + std::string(command_line_switch)))) {
    feature.enabled = false;
  } else if (command_line_->HasSwitch(
              ("enable-" + std::string(command_line_switch)))) {
    feature.enabled = true;
  } else {
    feature.enabled = (status == Stable);
  }

  runtimeFeatures_.push_back(feature);
}

bool XWalkRuntimeFeatures::isFeatureEnabled(const char* name) const {
  RuntimeFeaturesList::const_iterator it = std::find_if(
    runtimeFeatures_.begin(), runtimeFeatures_.end(),
      MatchRuntimeFeature(name));
  if (it == runtimeFeatures_.end())
    return false;
  return (*it).enabled;
}

}  // namespace xwalk
