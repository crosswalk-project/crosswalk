// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_PARSER_H_
#define XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_PARSER_H_

#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/application/common/tizen/signature_data.h"

namespace xwalk {
namespace application {

class SignatureParser {
 public:
  static scoped_ptr<SignatureData> CreateSignatureData(
      const base::FilePath& signature_path, int signature_number);

 private:
  DISALLOW_COPY_AND_ASSIGN(SignatureParser);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_PARSER_H_
