// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_I18N_DATA_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_I18N_DATA_H_

#include <string>
#include <vector>

#include "base/values.h"
#include "xwalk/application/common/manifest.h"

namespace xwalk {
namespace application {

class ManifestI18NData {
 public:
  ManifestI18NData();
  // Parse i18n element(s) from a path(the value of this path should
  // be DictionaryValue or ListValue).
  void ParseFromPath(const Manifest* manifest,
                     const std::string& path);

  // Get a local string according to the locale.
  bool GetString(const std::string& locale,
                 const std::string& key,
                 std::string* out_value) const;

  // Get a local string according to the following locale priority:
  // System locale (locale get from system).                      | high
  // Default locale (defaultlocale attribute of widget element)
  // Default (the element without xml:lang attribute)
  // Auto ("en-gb" will be considered as a default)               | low
  bool GetString(const std::string& key, std::string* out_value) const;

  void SetDefaultLocale(const std::string& default_locale) {
    default_locale_ = default_locale;
  }

 private:
  void ParseEachElement(const base::DictionaryValue* value,
                        const std::string& path);

  scoped_ptr<base::DictionaryValue> value_;
  std::string default_locale_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_I18N_DATA_H_
