// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_VALIDATOR_H_
#define XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_VALIDATOR_H_

#include "base/files/file_path.h"

namespace xwalk {
namespace application {

class SignatureValidator {
 public:
  enum Status {
    UNTRUSTED,
    VALID,
    INVALID
  };

  static Status Check(const base::FilePath& widget_path);

 private:
  DISALLOW_COPY_AND_ASSIGN(SignatureValidator);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_VALIDATOR_H_
