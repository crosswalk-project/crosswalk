// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_storage_constants.h"
#include "base/strings/stringprintf.h"

namespace xwalk {
namespace application_storage_constants {

const char kAppTableName[] = "applications";
const char kEventTableName[] = "registered_events";
const char kPermissionTableName[] = "stored_permissions";
const char kGarbageCollectionTableName[] = "garbage_collection";

const char kCreateAppTableOp[] =
    "CREATE TABLE applications ("
    "id TEXT NOT NULL UNIQUE PRIMARY KEY,"
    "manifest TEXT NOT NULL,"
    "path TEXT NOT NULL,"
    "install_time REAL)";

const char kCreateEventTableOp[] =
    "CREATE TABLE registered_events ("
    "id TEXT NOT NULL,"
    "event_names TEXT NOT NULL,"
    "PRIMARY KEY (id),"
    "FOREIGN KEY (id) REFERENCES applications(id)"
    "ON DELETE CASCADE)";

const char kCreatePermissionTableOp[] =
    "CREATE TABLE stored_permissions ("
    "id TEXT NOT NULL,"
    "permission_names TEXT NOT NULL,"
    "PRIMARY KEY (id),"
    "FOREIGN KEY (id) REFERENCES applications(id)"
    "ON DELETE CASCADE)";

const char kCreateGarbageCollectionTableOp[] =
    "CREATE TABLE garbage_collection ("
    "app_id TEXT NOT NULL PRIMARY KEY)";

const char kCreateGarbageCollectionTriggersOp[] =
    "CREATE TRIGGER IF NOT EXISTS add_garbage_app AFTER DELETE ON applications"
    " BEGIN INSERT INTO garbage_collection VALUES (OLD.id); END;"
    "CREATE TRIGGER IF NOT EXISTS del_garbage_app AFTER INSERT ON applications"
    " BEGIN DELETE FROM garbage_collection WHERE app_id = NEW.id; END";

const char kGetAllRowsFromAppEventTableOp[] =
    "SELECT A.id, A.manifest, A.path, A.install_time, "
    "B.event_names, C.permission_names "
    "FROM applications as A "
    "LEFT JOIN registered_events as B "
    "ON A.id = B.id "
    "LEFT JOIN stored_permissions as C "
    "ON A.id = C.id";

const char kSetApplicationWithBindOp[] =
    "INSERT INTO applications (manifest, path, install_time, id) "
    "VALUES (?,?,?,?)";

const char kUpdateApplicationWithBindOp[] =
    "UPDATE applications SET manifest = ?, path = ?,"
    "install_time = ? WHERE id = ?";

const char kDeleteApplicationWithBindOp[] =
    "DELETE FROM applications WHERE id = ?";

const char kInsertEventsWithBindOp[] =
    "INSERT INTO registered_events (event_names, id) "
    "VALUES(?,?)";

const char kUpdateEventsWithBindOp[] =
    "UPDATE registered_events SET event_names = ? WHERE id = ?";

const char kDeleteEventsWithBindOp[] =
    "DELETE FROM registered_events WHERE id = ?";

const char kInsertPermissionsWithBindOp[] =
    "INSERT INTO stored_permissions (permission_names, id) "
    "VALUES(?,?)";

const char kUpdatePermissionsWithBindOp[] =
    "UPDATE stored_permissions SET permission_names = ? WHERE id = ?";

const char kDeletePermissionsWithBindOp[] =
    "DELETE FROM stored_permissions WHERE id = ?";

const char kGetAllRowsFromGarbageCollectionTableOp[] =
    "SELECT app_id FROM garbage_collection";

const char kDeleteGarbageAppIdWithBindOp[] =
    "DELETE FROM garbage_collection WHERE app_id = ?";

}  // namespace application_storage_constants
}  // namespace xwalk
