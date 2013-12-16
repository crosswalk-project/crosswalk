// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_storage_constants.h"
#include "base/strings/stringprintf.h"

namespace xwalk {
namespace application_storage_constants {

const char kAppTableName[] = "applications";
const char kEventTableName[] = "registered_events";

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

const char kGetAllRowsFromAppEventTableOp[] =
    "SELECT A.id, A.manifest, A.path, A.install_time, B.event_names "
    "FROM applications as A "
    "LEFT JOIN registered_events as B "
    "ON A.id = B.id";

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

}  // namespace application_storage_constants
}  // namespace xwalk
