// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/install_warning.h"

namespace xwalk{
namespace application {

void PrintTo(const InstallWarning& warning, ::std::ostream* os) {
  *os << "InstallWarning(";
  switch (warning.format) {
    case InstallWarning::FORMAT_TEXT:
      *os << "FORMAT_TEXT, \"";
      break;
    case InstallWarning::FORMAT_HTML:
      *os << "FORMAT_HTML, \"";
      break;
  }
  // This is just for test error messages, so no need to escape '"'
  // characters inside the message.
  *os << warning.message << "\")";
}

}  // namespace application
}  // namespace xwalk
