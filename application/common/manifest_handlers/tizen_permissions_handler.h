// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_PERMISSIONS_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_PERMISSIONS_HANDLER_H_

#include <string>
#include <vector>

#include "xwalk/application/common/manifest_handler.h"
#include "xwalk/application/common/permission_types.h"

namespace xwalk {
namespace application {

class TizenPermissionsHandler: public ManifestHandler {
 public:
  TizenPermissionsHandler();
  virtual ~TizenPermissionsHandler();

  bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) override;
  bool AlwaysParseForType(Manifest::Type type) const override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TizenPermissionsHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_TIZEN_PERMISSIONS_HANDLER_H_
