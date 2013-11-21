// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_DB_STORE_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_DB_STORE_CONSTANTS_H_

#include "base/files/file_path.h"
#include "base/basictypes.h"

namespace xwalk {
namespace db_store_constants {
  extern const char kAppTableName[];
  extern const char kAppID[];
  extern const char kAppManifest[];
  extern const char kAppPath[];
  extern const char kAppInstallTime[];

  extern const char kEventTableName[];
  extern const char kEventAppID[];
  extern const char kEventsName[];

  extern const char kCreateAppTableOp[];
  extern const char kCreateEventTableOp[];
  extern const char kGetAllRowsFromAppEventTableOp[];
  extern const char kSetApplicationWithBindOp[];
  extern const char kUpdateApplicationWithBindOp[];
  extern const char kDeleteApplicationWithBindOp[];
  extern const char kSetManifestWithBindOp[];
  extern const char kSetInstallTimeWithBindOp[];
  extern const char kSetApplicationPathWithBindOp[];
  extern const char kInsertEventsWithBindOp[];
  extern const char kUpdateEventsWithBindOp[];
  extern const char kDeleteEventsWithBindOp[];

}  // namespace db_store_constants
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_DB_STORE_CONSTANTS_H_
