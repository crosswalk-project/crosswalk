// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_CSP_HANDLER_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_CSP_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/manifest_handler.h"

namespace xwalk {
namespace application {

class CSPInfo : public ApplicationData::ManifestData {
 public:
  CSPInfo();
  virtual ~CSPInfo();

  void SetDirective(const std::string& directive_name,
                    const std::vector<std::string>& directive_value) {
    policies_[directive_name] = directive_value;
  }
  const std::map<std::string, std::vector<std::string> >&
      GetDirectives() const { return policies_; }

 private:
  std::map<std::string, std::vector<std::string> > policies_;
};

class CSPHandler : public ManifestHandler {
 public:
  CSPHandler();
  virtual ~CSPHandler();

  virtual bool Parse(scoped_refptr<ApplicationData> application,
                     string16* error) OVERRIDE;
  virtual bool AlwaysParseForType(Manifest::Type type) const OVERRIDE;
  virtual std::vector<std::string> Keys() const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(CSPHandler);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_CSP_HANDLER_H_
