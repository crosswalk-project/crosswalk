// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_CONTENT_CLIENT_H_
#define XWALK_RUNTIME_COMMON_XWALK_CONTENT_CLIENT_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "content/public/common/content_client.h"

namespace xwalk {

std::string GetUserAgent();

class XWalkContentClient : public content::ContentClient {
 public:
  XWalkContentClient();
  ~XWalkContentClient() override;

  static const char* const kNaClPluginName;

  void AddPepperPlugins(
      std::vector<content::PepperPluginInfo>* plugins) override;
  std::string GetProduct() const override;
  std::string GetUserAgent() const override;
  base::string16 GetLocalizedString(int message_id) const override;
  base::StringPiece GetDataResource(
      int resource_id,
      ui::ScaleFactor scale_factor) const override;
  base::RefCountedMemory* GetDataResourceBytes(
      int resource_id) const override;
  gfx::Image& GetNativeImageNamed(int resource_id) const override;
  void AddAdditionalSchemes(
      std::vector<url::SchemeWithType>* standard_schemes,
      std::vector<url::SchemeWithType>* referrer_schemes,
      std::vector<std::string>* saveable_shemes) override;
  void AddSecureSchemesAndOrigins(std::set<std::string>* schemes,
      std::set<GURL>* origins) override;
  std::string GetProcessTypeNameInEnglish(int type) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkContentClient);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_COMMON_XWALK_CONTENT_CLIENT_H_
