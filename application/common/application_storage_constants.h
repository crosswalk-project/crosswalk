// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_CONSTANTS_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_CONSTANTS_H_

#include "base/files/file_path.h"
#include "base/basictypes.h"

namespace xwalk {
namespace application_storage_constants {
  extern const char kAppTableName[];
  extern const char kPermissionTableName[];
  extern const char kGarbageCollectionTableName[];

  extern const char kCreateAppTableOp[];
  extern const char kCreatePermissionTableOp[];
  extern const char kCreateGarbageCollectionTableOp[];
  extern const char kCreateGarbageCollectionTriggersOp[];
  extern const char kGetRowFromAppTableOp[];
  extern const char kGetAllRowsFromAppTableOp[];
  extern const char kSetApplicationWithBindOp[];
  extern const char kUpdateApplicationWithBindOp[];
  extern const char kDeleteApplicationWithBindOp[];
  extern const char kInsertPermissionsWithBindOp[];
  extern const char kUpdatePermissionsWithBindOp[];
  extern const char kDeletePermissionsWithBindOp[];
  extern const char kGetAllRowsFromGarbageCollectionTableOp[];
  extern const char kDeleteGarbageAppIdWithBindOp[];

}  // namespace application_storage_constants
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_STORAGE_CONSTANTS_H_
