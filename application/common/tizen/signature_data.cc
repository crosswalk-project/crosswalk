// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/tizen/signature_data.h"

#include "base/strings/utf_string_conversions.h"

namespace xwalk {
namespace application {

SignatureData::SignatureData(const std::string& signature_file_name,
    int signature_number)
    : signature_file_name_(signature_file_name),
      signature_number_(signature_number) {
}

SignatureData::~SignatureData() {
}

base::FilePath SignatureData::GetExtractedWidgetPath() const {
  std::string widget_path = signature_file_name();
  size_t pos = widget_path.rfind('/');
  if (pos == std::string::npos) {
    widget_path.clear();
  } else {
    widget_path.erase(pos + 1, std::string::npos);
  }
#if defined (OS_WIN)
  return base::FilePath(base::UTF8ToWide(widget_path));
#else
  return base::FilePath(widget_path);
#endif
}

}  // namespace application
}  // namespace xwalk
