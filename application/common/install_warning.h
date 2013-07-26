// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_INSTALL_WARNING_H_
#define XWALK_APPLICATION_COMMON_INSTALL_WARNING_H_

#include <ostream>
#include <string>

namespace xwalk_application {

struct InstallWarning {
  enum Format {
    // IMPORTANT: Do not build HTML strings from user or developer-supplied
    // input.
    FORMAT_TEXT,
    FORMAT_HTML,
  };
  InstallWarning(Format format, const std::string& message)
      : format(format), message(message) {
  }
  bool operator==(const InstallWarning& other) const {
    return format == other.format && message == other.message;
  }
  Format format;
  std::string message;
};

// Let gtest print InstallWarnings.
void PrintTo(const InstallWarning&, ::std::ostream* os);

}  // namespace xwalk_application

#endif  // XWALK_APPLICATION_COMMON_INSTALL_WARNING_H_
