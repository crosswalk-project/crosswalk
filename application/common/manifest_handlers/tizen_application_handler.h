// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APPLICATION_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APPLICATION_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "base/values.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class TizenApplicationInfo : public ApplicationData::ManifestData {
 public:
  TizenApplicationInfo();
  virtual ~TizenApplicationInfo();

  void set_id(const std::string& id) {
    id_ = id;
  }
  void set_package(const std::string& package) {
    package_ = package;
  }
  void set_required_version(
      const std::string& required_version) {
    required_version_ = required_version;
  }
  const std::string& id() const {
    return id_;
  }
  const std::string& package() const {
    return package_;
  }
  const std::string& required_version() const {
    return required_version_;
  }

 private:
  std::string id_;
  std::string package_;
  std::string required_version_;
};

class TizenApplicationHandler : public ManifestHandler {
 public:
  TizenApplicationHandler();
  virtual ~TizenApplicationHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) OVERRIDE;
  virtual bool Validate(scoped_refptr<const ApplicationData> application,
                        std::string* error,
                        std::vector<InstallWarning>* warnings) const OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenApplicationHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_APPLICATION_HANDLER_H_
