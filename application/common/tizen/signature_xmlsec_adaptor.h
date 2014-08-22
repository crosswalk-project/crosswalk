// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_XMLSEC_ADAPTOR_H_
#define XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_XMLSEC_ADAPTOR_H_

#include "base/files/file_path.h"
#include "xwalk/application/common/tizen/signature_data.h"

namespace xwalk {
namespace application {

class SignatureXmlSecAdaptor {
 public:
  static bool ValidateFile(const SignatureData& signature_data,
                           const base::FilePath& widget_path);

 private:
  DISALLOW_COPY_AND_ASSIGN(SignatureXmlSecAdaptor);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_XMLSEC_ADAPTOR_H_
