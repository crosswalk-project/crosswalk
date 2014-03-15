// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WARP_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WARP_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class WARPInfo : public ApplicationData::ManifestData {
 public:
  WARPInfo();
  virtual ~WARPInfo();

  void SetWARP(const base::ListValue* warp) {
    warp_.reset(warp);
  }
  const base::ListValue* GetWARP() const { return warp_.get(); }

 private:
  scoped_ptr<const base::ListValue> warp_;
};

class WARPHandler : public ManifestHandler {
 public:
  WARPHandler();
  virtual ~WARPHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     base::string16* error) OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(WARPHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WARP_HANDLER_H_
