// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_system_locale.h"

namespace xwalk {
namespace {
  const char kDefaultLocale[] = "en-US";
}  // namespace

std::string GetSystemLocale() {
  return kDefaultLocale;
}

const char kIntlAcceptLanguage[] = "intl.accept_languages";

}  // namespace xwalk
