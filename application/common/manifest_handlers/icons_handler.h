// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_ICONS_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_ICONS_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "base/values.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class IconsInfo : public ApplicationData::ManifestData {
 public:
  IconsInfo();
  virtual ~IconsInfo();
  void SetIcon(uint32 size, const base::FilePath& src);

  const std::map<uint32, base::FilePath>& GetIcons() { return icons_;}

 private:
  std::map<uint32, base::FilePath> icons_;
};

class IconsHandler : public ManifestHandler {
 public:
  explicit IconsHandler(Package::Type type);
  virtual ~IconsHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  Package::Type package_type_;

  DISALLOW_COPY_AND_ASSIGN(IconsHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_ICONS_HANDLER_H_
