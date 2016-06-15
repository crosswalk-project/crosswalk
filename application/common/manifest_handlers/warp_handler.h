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
  ~WARPInfo() override;

  void SetWARP(const base::ListValue* warp) {
    warp_.reset(warp);
  }
  const base::ListValue* GetWARP() const { return warp_.get(); }

 private:
  std::unique_ptr<const base::ListValue> warp_;
};

class WARPHandler : public ManifestHandler {
 public:
  WARPHandler();
  ~WARPHandler() override;

  bool Parse(scoped_refptr<ApplicationData> application,
             base::string16* error) override;
  std::vector<std::string> Keys() const override;

 private:
  DISALLOW_COPY_AND_ASSIGN(WARPHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_WARP_HANDLER_H_
