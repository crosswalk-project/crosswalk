// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_ID_UTIL_H_
#define XWALK_APPLICATION_COMMON_ID_UTIL_H_

#include <string>

#include "xwalk/application/common/application_data.h"

namespace base {
class FilePath;
}

namespace xwalk {
namespace application {

// The number of bytes in a legal id.
extern const size_t kIdSize;

#if defined(OS_TIZEN)
// The number of bytes in a legal legacy Tizen id.
extern const size_t kLegacyTizenIdSize;
#endif

// Generates an application ID from arbitrary input. The same input string will
// always generate the same output ID.
std::string GenerateId(const std::string& input);

// Generate an ID for an application in the given path.
// Used while developing applications, before they have a key.
std::string GenerateIdForPath(const base::FilePath& path);

#if defined(OS_TIZEN)
// If this appid is a xpk app id(crosswalk_32bytes_app_id), return itself.
// If this appid is a wgt app id(tizen_app_id), convert it to
// crosswalk_32bytes_app_id and return it.
std::string RawAppIdToCrosswalkAppId(const std::string& id);

// If this appid is a xpk app id(crosswalk_32bytes_app_id), return
// xwalk.crosswalk_32bytes_app_id.
// If this appid is a wgt app id(tizen_app_id), return itself.
// It is better to storage crosswalk_32bytes_app_id on tizen pkgmgr db
// for xpk, but it must be an "." on appid or it cannot insert to tizen pkgmgr
// db, so we have to have a "xwalk." as it's prefix.
std::string RawAppIdToAppIdForTizenPkgmgrDB(const std::string& id);

// For xpk, app_id == crosswalk_32bytes_app_id == this->ID(),
// For wgt, app_id == tizen_wrt_10bytes_package_id.app_name,
std::string GetTizenAppId(ApplicationData* application);
#endif

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_ID_UTIL_H_
