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

XWalkRuntimeFeatures::Feature::Feature()
    : status(Experimental),
      enabled(false) {
}

XWalkRuntimeFeatures::Feature::Feature(
    const std::string& name,
    const std::string& cmd_line,
    const std::string& description,
    Status status,
    bool enabled)
    : name(name),
      cmd_line(cmd_line),
      description(description),
      status(status),
      enabled(enabled) {
}

XWalkRuntimeFeatures::Feature::Feature(const XWalkRuntimeFeatures::Feature&) =
    default;

// static
XWalkRuntimeFeatures* XWalkRuntimeFeatures::GetInstance() {
  return base::Singleton<XWalkRuntimeFeatures>::get();
}

XWalkRuntimeFeatures::XWalkRuntimeFeatures()
  : command_line_(0),
    initialized_(false),
    experimental_features_enabled_(false) {}

void XWalkRuntimeFeatures::Initialize(const base::CommandLine* cmd) {
  command_line_ = cmd;
  initialized_ = true;
  runtime_features_.clear();
  if (cmd->HasSwitch(switches::kExperimentalFeatures))
    experimental_features_enabled_ = true;
  else
    experimental_features_enabled_ = false;

#if !defined(DISABLE_BUNDLED_EXTENSIONS)
  const Feature::Status default_status = Feature::Stable;
#else
  // mark all features as experimental so they are disabled by default
  const Feature::Status default_status = Feature::Experimental;
#endif

#if defined(OS_ANDROID)
  // FIXME(cmarcelo): The application extensions are currently not fully working
  // for Android, so disable them. See
  // https://crosswalk-project.org/jira/browse/XWALK-674.
  // Android uses a Java extension for device capabilities, so disable the one
  // from sysapps/.
  AddFeature("SysApps", "sysapps",
      "Master switch for the SysApps category of APIs", default_status);
  AddFeature("RawSocketsAPI", "raw-sockets",
      "JavaScript support for using TCP and UDP sockets", default_status);
  AddFeature("StorageAPI", "storage",
      "JavaScript support to file system beyond W3C spec", default_status);
  AddFeature("DialogAPI", "dialog-api",
      "JavaScript support for dialog APIs", Feature::Experimental);
#else
  AddFeature("SysApps", "sysapps",
      "Master switch for the SysApps category of APIs", default_status);
  AddFeature("RawSocketsAPI", "raw-sockets",
      "JavaScript support for using TCP and UDP sockets", default_status);
  AddFeature("StorageAPI", "storage",
      "JavaScript support to file system beyond W3C spec", default_status);
  AddFeature("ApplicationAPI", "application-api",
      "JavaScript support for Widget and Manifest APIs", default_status);
  AddFeature("DialogAPI", "dialog-api",
      "JavaScript support for dialog APIs", Feature::Experimental);
  AddFeature("WiFiDirectAPI", "wifidirect-api",
      "JavaScript support for WiFiDirect", Feature::Experimental);
#endif
}

XWalkRuntimeFeatures::~XWalkRuntimeFeatures() {}

void XWalkRuntimeFeatures::AddFeature(const char* name,
                                      const char* cmd_line,
                                      const char* description,
                                      Feature::Status status) {
  Feature feature(name, cmd_line, description, status);
  if (experimental_features_enabled_) {
    feature.enabled = true;
  } else if (command_line_->HasSwitch("disable-" + std::string(cmd_line))) {
    feature.enabled = false;
  } else if (command_line_->HasSwitch("enable-" + std::string(cmd_line))) {
    feature.enabled = true;
  } else {
    feature.enabled = (status == Feature::Stable);
  }

  runtime_features_[name] = feature;
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

  for (const auto& pair : runtime_features_) {
    const Feature& feature = pair.second;
    std::string status = (feature.status == Feature::Stable) ?
      std::string("Stable") : std::string("Experimental");
    std::string command_line;

    feature.enabled ? command_line = "--disable-"
      : command_line = "--enable-";
    command_line += feature.cmd_line;

    output += command_line;
    std::string space(command_line_description_space
      + command_line_title.length() - command_line.length(), ' ');
    output +=  space + feature.description;
    std::string space2(description_status_space + description_title.length()
      - feature.description.length(), ' ');
    output += space2 + status + '\n';
  }
  std::cout << output << std::endl;
}

bool XWalkRuntimeFeatures::isFeatureEnabled(const char* name) const {
  CHECK(initialized_);
  auto it = runtime_features_.find(name);
  if (it == runtime_features_.end())
    return false;
  return it->second.enabled;
}

}  // namespace xwalk
