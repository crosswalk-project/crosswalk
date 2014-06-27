// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_storage_constants.h"
#include "base/strings/stringprintf.h"

namespace xwalk {
namespace application_storage_constants {

const char kAppTableName[] = "applications";
const char kPermissionTableName[] = "stored_permissions";
const char kGarbageCollectionTableName[] = "garbage_collection";

const char kCreateAppTableOp[] =
    "CREATE TABLE applications ("
    "id TEXT NOT NULL UNIQUE PRIMARY KEY,"
    "manifest TEXT NOT NULL,"
    "path TEXT NOT NULL,"
    "install_time REAL)";

const char kCreatePermissionTableOp[] =
    "CREATE TABLE stored_permissions ("
    "id TEXT NOT NULL,"
    "permission_names TEXT NOT NULL,"
    "PRIMARY KEY (id),"
    "FOREIGN KEY (id) REFERENCES applications(id)"
    "ON DELETE CASCADE)";

const char kGetRowFromAppTableOp[] =
    "SELECT A.id, A.manifest, A.path, A.install_time, "
    "C.permission_names FROM applications as A "
    "LEFT JOIN stored_permissions as C "
    "ON A.id = C.id WHERE A.id = ?";

const char kGetAllRowsFromAppTableOp[] =
    "SELECT A.id, A.manifest, A.path, A.install_time, "
    "C.permission_names FROM applications as A "
    "LEFT JOIN stored_permissions as C "
    "ON A.id = C.id";

extern const char kGetAllIDsFromAppTableOp[] =
    "SELECT id FROM applications";

const char kSetApplicationWithBindOp[] =
    "INSERT INTO applications (manifest, path, install_time, id) "
    "VALUES (?,?,?,?)";

const char kUpdateApplicationWithBindOp[] =
    "UPDATE applications SET manifest = ?, path = ?,"
    "install_time = ? WHERE id = ?";

const char kDeleteApplicationWithBindOp[] =
    "DELETE FROM applications WHERE id = ?";

const char kInsertPermissionsWithBindOp[] =
    "INSERT INTO stored_permissions (permission_names, id) "
    "VALUES(?,?)";

const char kUpdatePermissionsWithBindOp[] =
    "UPDATE stored_permissions SET permission_names = ? WHERE id = ?";

const char kDeletePermissionsWithBindOp[] =
    "DELETE FROM stored_permissions WHERE id = ?";

}  // namespace application_storage_constants
}  // namespace xwalk
