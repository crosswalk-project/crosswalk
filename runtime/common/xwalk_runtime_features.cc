// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_runtime_features.h"

#include <algorithm>
#include <functional>
#include <iostream> // NOLINT

#include "base/logging.h"
#include "xwalk/runtime/common/xwalk_switches.h"

namespace xwalk {

struct MatchRuntimeFeature
  : std::unary_function<XWalkRuntimeFeatures::RuntimeFeature, bool> {
  explicit MatchRuntimeFeature(const std::string& name) : name(name) {}
  bool operator()(const XWalkRuntimeFeatures::RuntimeFeature& entry) const {
    return entry.name == name;
  }
  const std::string name;
};

XWalkRuntimeFeatures::RuntimeFeature::RuntimeFeature() {}

// static
XWalkRuntimeFeatures* XWalkRuntimeFeatures::GetInstance() {
  return Singleton<XWalkRuntimeFeatures>::get();
}

XWalkRuntimeFeatures::XWalkRuntimeFeatures()
  : command_line_(0)
  , initialized_(false) {}

void XWalkRuntimeFeatures::Initialize(const CommandLine* cmd) {
  command_line_ = cmd;
  initialized_ = true;
  runtime_features_.clear();
  if (cmd->HasSwitch(switches::kExperimentalFeatures))
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

void XWalkRuntimeFeatures::AddFeature(const char* name,
                                 const char* command_line_switch,
                                 const char* description,
                                 RuntimeFeatureStatus status) {
  RuntimeFeature feature;
  feature.name = name;
  feature.description = description;
  feature.command_line_switch = command_line_switch;
  feature.status = status;

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

void XWalkRuntimeFeatures::DumpFeaturesFlags() {
  std::cout << "Available runtime features flags : " << std::endl;
  const std::string command_line_title("  Command Line Switch");
  const std::string description_title("Description");
  const std::string status_title("Status");
  const int command_line_description_space = 16;
  const int description_status_space = 51;
  const int status_space = 7;

  std::string output;
  output += command_line_title;
  output += std::string(command_line_description_space, ' ');
  output += description_title;
  output += std::string(description_status_space, ' ');
  output += status_title + std::string(status_space, ' ') + '\n';
  int total_length = status_space + description_status_space
    + command_line_description_space + description_title.length()
    + command_line_title.length() + status_title.length();
  output += std::string(total_length, '-') + '\n';

  RuntimeFeaturesList::const_iterator it = runtime_features_.begin();
  for (; it != runtime_features_.end(); ++it) {
    std::string status = (it->status == Stable) ?
      std::string("Stable") : std::string("Experimental");
    std::string command_line;

    it->enabled ? command_line = "--disable-"
      : command_line = "--enable-";
    command_line += it->command_line_switch;

    output += command_line;
    std::string space(command_line_description_space
      + command_line_title.length() - command_line.length(), ' ');
    output +=  space + it->description;
    std::string space2(description_status_space + description_title.length()
      - it->description.length(), ' ');
    output += space2 + status + '\n';
  }
  std::cout << output << std::endl;
}

bool XWalkRuntimeFeatures::isFeatureEnabled(const char* name) const {
  CHECK(initialized_);
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
