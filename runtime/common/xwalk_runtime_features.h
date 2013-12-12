// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_RUNTIME_FEATURES_H_
#define XWALK_RUNTIME_COMMON_XWALK_RUNTIME_FEATURES_H_

#include <string>
#include <vector>

#include "base/command_line.h"

namespace xwalk {

#define DECLARE_RUNTIME_FEATURE(NAME) static bool is ##NAME## Enabled() \
  { return GetInstance()->isFeatureEnabled( #NAME ); }

class XWalkRuntimeFeatures {
 public:
  // Declare new features here and and define them in xwalk_runtime_features.cc.
  DECLARE_RUNTIME_FEATURE(RawSocketsAPI);
  DECLARE_RUNTIME_FEATURE(DeviceCapabilitiesAPI);
  DECLARE_RUNTIME_FEATURE(DialogAPI);

  static void Initialize(const CommandLine* cmd);
  static void DumpFeaturesFlagsInCommandLine();
  static XWalkRuntimeFeatures* GetInstance();

  enum RuntimeFeatureStatus {
    Stable,
    Experimental
  };


  struct RuntimeFeature {
    std::string name;
    std::string description;
    std::string command_line_switch;
    RuntimeFeatureStatus status;
    bool enabled;
    RuntimeFeature();
  };

 private:
  explicit XWalkRuntimeFeatures(const CommandLine* cmd);
  ~XWalkRuntimeFeatures();
  void AddFeature(const char* name, const char* command_line_switch,
                  const char* description, RuntimeFeatureStatus status);
  void DumpFeaturesFlags();
  bool isFeatureEnabled(const char* name) const;
  typedef std::vector<RuntimeFeature> RuntimeFeaturesList;
  RuntimeFeaturesList runtime_features_;
  const CommandLine* command_line_;
  bool experimental_features_enabled_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_COMMON_XWALK_RUNTIME_FEATURES_H_
