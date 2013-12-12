// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_runtime_features.h"

#include <algorithm>
#include <functional>
#include <iostream>

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
  if (cmd->HasSwitch("enable-xwalk-experimental-features"))
    experimental_features_enabled_ = true;
  else
    experimental_features_enabled_ = false;
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
  AddFeature("DialogAPI", "dialog",
             "JavaScript support to create open/save native dialogs"
             , Experimental);
}

XWalkRuntimeFeatures::~XWalkRuntimeFeatures() {}

void XWalkRuntimeFeatures::DumpFeatures() const {
  std::cout << "Runtime features:" << std::endl;
  for (RuntimeFeaturesList::const_iterator it = runtime_features_.begin();
       it != runtime_features_.end(); ++it) {
    std::cout
        << (it->enabled ? "  --disable-" : "  --enable-")
        << it->command_line_switch
        << (it->enabled ? " (disable " : " (enable ") << it->description << ")"
        << std::endl;
  }
}

void XWalkRuntimeFeatures::AddFeature(const char* name,
                                 const char* command_line_switch,
                                 const char* description,
                                 RuntimeFeatureStatus status) {
  RuntimeFeature feature;
  feature.name = name;
  feature.command_line_switch = command_line_switch;
  feature.description = description;

  if (experimental_features_enabled_) {
    feature.enabled = true;
  } else if (command_line_->HasSwitch(
              ("disable-" + std::string(command_line_switch)))) {
    feature.enabled = false;
  } else if (command_line_->HasSwitch(
              ("enable-" + std::string(command_line_switch)))) {
    feature.enabled = true;
  } else {
    feature.enabled = (status == Stable);
  }

  runtime_features_.push_back(feature);
}

bool XWalkRuntimeFeatures::isFeatureEnabled(const char* name) const {
  if (experimental_features_enabled_)
    return true;

  RuntimeFeaturesList::const_iterator it = std::find_if(
    runtime_features_.begin(), runtime_features_.end(),
      MatchRuntimeFeature(name));
  if (it == runtime_features_.end())
    return false;
  return (*it).enabled;
}

}  // namespace xwalk
