// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/installer/signature_data.h"

namespace xwalk {
namespace application {

SignatureData::SignatureData(const std::string& signature_file_name,
    int signature_number)
    : signature_file_name_(signature_file_name),
      signature_number_(signature_number) {
}

SignatureData::~SignatureData() {
}

}  // namespace application
}  // namespace xwalk
