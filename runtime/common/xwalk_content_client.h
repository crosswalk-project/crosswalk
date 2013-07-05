// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_RUNTIME_COMMON_XWALK_CONTENT_CLIENT_H_
#define CAMEO_RUNTIME_COMMON_XWALK_CONTENT_CLIENT_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "content/public/common/content_client.h"

namespace xwalk {

class XWalkContentClient : public content::ContentClient {
 public:
  XWalkContentClient();
  virtual ~XWalkContentClient();

  virtual std::string GetUserAgent() const OVERRIDE;
  virtual string16 GetLocalizedString(int message_id) const OVERRIDE;
  virtual base::StringPiece GetDataResource(
      int resource_id,
      ui::ScaleFactor scale_factor) const OVERRIDE;
  virtual base::RefCountedStaticMemory* GetDataResourceBytes(
      int resource_id) const OVERRIDE;
  virtual gfx::Image& GetNativeImageNamed(int resource_id) const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkContentClient);
};

}  // namespace xwalk

#endif  // CAMEO_RUNTIME_COMMON_XWALK_CONTENT_CLIENT_H_
