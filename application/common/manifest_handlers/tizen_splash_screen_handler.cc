// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/tizen_splash_screen_handler.h"

#include <map>
#include <utility>

#include "base/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {

TizenSplashScreenInfo::TizenSplashScreenInfo() {}
TizenSplashScreenInfo::~TizenSplashScreenInfo() {}

TizenSplashScreenHandler::TizenSplashScreenHandler() {}

TizenSplashScreenHandler::~TizenSplashScreenHandler() {}

bool TizenSplashScreenHandler::Parse(scoped_refptr<ApplicationData> application,
                                     base::string16* error) {
  scoped_ptr<TizenSplashScreenInfo> ss_info(new TizenSplashScreenInfo);
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);

  base::Value* splash_screen = NULL;
  manifest->Get(keys::kTizenSplashScreenKey, &splash_screen);
  if (splash_screen && splash_screen->IsType(base::Value::TYPE_DICTIONARY)) {
    base::DictionaryValue* ss_dict = NULL;
    splash_screen->GetAsDictionary(&ss_dict);
    std::string src;
    ss_dict->GetString(keys::kTizenSplashScreenSrcKey, &src);
    ss_info->set_src(src);
  }
  application->SetManifestData(keys::kTizenSplashScreenKey, ss_info.release());
  return true;
}

bool TizenSplashScreenHandler::Validate(
    scoped_refptr<const ApplicationData> application,
    std::string* error) const {
  const Manifest* manifest = application->GetManifest();
  DCHECK(manifest);
  base::Value* splash_screen = NULL;
  manifest->Get(keys::kTizenSplashScreenKey, &splash_screen);
  if (!splash_screen || !splash_screen->IsType(base::Value::TYPE_DICTIONARY)) {
    *error = std::string("The splash-screen attribute is not set correctly.");
    return false;
  }
  base::DictionaryValue* ss_dict = NULL;
  splash_screen->GetAsDictionary(&ss_dict);
  std::string ss_src;
  ss_dict->GetString(keys::kTizenSplashScreenSrcKey, &ss_src);
  base::FilePath path = application->path().Append(FILE_PATH_LITERAL(ss_src));
  if (!base::PathExists(path)) {
    *error = std::string("The splash screen image does not exist");
    return false;
  }
  return true;
}

std::vector<std::string> TizenSplashScreenHandler::Keys() const {
  return std::vector<std::string>(1, keys::kTizenSplashScreenKey);
}

}  // namespace application
}  // namespace xwalk
