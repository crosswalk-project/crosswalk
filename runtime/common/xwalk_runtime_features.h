// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_RUNTIME_FEATURES_H_
#define XWALK_RUNTIME_COMMON_XWALK_RUNTIME_FEATURES_H_

#include <string>
#include <map>
#include <vector>

#include "base/command_line.h"
#include "base/memory/singleton.h"

namespace xwalk {

#define DECLARE_RUNTIME_FEATURE(NAME) static bool is ##NAME## Enabled() \
  { return GetInstance()->isFeatureEnabled( #NAME ); }

class XWalkRuntimeFeatures {
 public:
  // Declare new features here and and define them in xwalk_runtime_features.cc.
  DECLARE_RUNTIME_FEATURE(SysApps);
  DECLARE_RUNTIME_FEATURE(RawSocketsAPI);
  DECLARE_RUNTIME_FEATURE(StorageAPI);
  DECLARE_RUNTIME_FEATURE(DialogAPI);
  DECLARE_RUNTIME_FEATURE(ApplicationAPI);
  DECLARE_RUNTIME_FEATURE(WiFiDirectAPI);

  void Initialize(const base::CommandLine* cmd);
  void DumpFeaturesFlags();
  static XWalkRuntimeFeatures* GetInstance();

  struct Feature {
    enum Status {
      Stable,
      Experimental
    };

    Feature();
    Feature(const std::string& name,
            const std::string& cmd_line,
            const std::string& description,
            Status status = Experimental,
            bool enabled = false);
    Feature(const Feature&);

    std::string name;
    std::string cmd_line;
    std::string description;
    Status status;
    bool enabled;
  };

 private:
  friend struct base::DefaultSingletonTraits<XWalkRuntimeFeatures>;

  XWalkRuntimeFeatures();
  ~XWalkRuntimeFeatures();

  void AddFeature(const char* name,
                  const char* cmd_line,
                  const char* description,
                  Feature::Status status);
  bool isFeatureEnabled(const char* name) const;
  typedef std::map<std::string, Feature> RuntimeFeaturesMap;
  RuntimeFeaturesMap runtime_features_;
  const base::CommandLine* command_line_;
  bool initialized_;
  bool experimental_features_enabled_;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_COMMON_XWALK_RUNTIME_FEATURES_H_
