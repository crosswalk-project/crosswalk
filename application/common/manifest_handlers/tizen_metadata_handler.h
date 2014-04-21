// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_METADATA_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_METADATA_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "base/values.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class TizenMetaDataInfo : public ApplicationData::ManifestData {
 public:
  TizenMetaDataInfo();
  virtual ~TizenMetaDataInfo();

  bool HasKey(const std::string& key) const;
  std::string GetValue(const std::string& key) const;
  void SetValue(const std::string& key, const std::string& value);
  const std::map<std::string, std::string>& metadata() const {
    return metadata_;
  }

 private:
  std::map<std::string, std::string> metadata_;
};

class TizenMetaDataHandler : public ManifestHandler {
 public:
  TizenMetaDataHandler();
  virtual ~TizenMetaDataHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenMetaDataHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_METADATA_HANDLER_H_
