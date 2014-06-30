// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_system_locale.h"

#if defined(OS_TIZEN)
#include <vconf.h>
#include <algorithm>

#include "base/logging.h"
#endif

namespace xwalk {
#if defined(OS_TIZEN)

namespace {
const char kTizenDefaultLocale[] = "en-GB";

bool TizenLocaleToBCP47Locale(const std::string& tizen_locale,
                              std::string* out_BCP47_locale) {
  if (tizen_locale.empty())
    return false;

  // Tizen locale format [language[_territory][.codeset][@modifier]] .
  // Conver to BCP47 format language[-territory] .
  *out_BCP47_locale = tizen_locale.substr(0, tizen_locale.find_first_of("."));
  std::replace(out_BCP47_locale->begin(), out_BCP47_locale->end(), '_', '-');
  return true;
}

}  // namespace

std::string GetSystemLocale() {
  std::string tizen_locale;
  char* langset = vconf_get_str(VCONFKEY_LANGSET);
  if (langset) {
    tizen_locale = langset;
  } else {
    LOG(ERROR) << "Can not get VCONFKEY_LANGSET from vconf or "
               << "VCONFKEY_LANGSET vlaue is not a string value";
  }
  free(langset);

  // Tizen take en-GB as default.
  std::string BCP47_locale(kTizenDefaultLocale);
  TizenLocaleToBCP47Locale(tizen_locale, &BCP47_locale);
  return BCP47_locale;
}
#else
namespace {
  const char kDefaultLocale[] = "en-US";
}  // namespace

std::string GetSystemLocale() {
  return kDefaultLocale;
}

#endif

}  // namespace xwalk
