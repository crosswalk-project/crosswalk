// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/db_store_constants.h"
#include "base/strings/stringprintf.h"

namespace xwalk {
namespace db_store_constants {

const char kAppTableName[] = "applications";
const char kAppID[] = "id";
const char kAppManifest[] = "manifest";
const char kAppPath[] = "path";
const char kAppInstallTime[] = "install_time";

const char kEventTableName[] = "registered_events";
const char kEventAppID[] = "id";
const char kEventsName[] = "event_names";

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
    "SELECT applications.id, manifest, path, install_time, event_names "
    "FROM applications "
    "LEFT JOIN registered_events "
    "ON applications.id = registered_events.id";

const char kSetApplicationWithBindOp[] =
    "INSERT INTO applications (manifest, path, install_time, id) "
    "VALUES (?,?,?,?)";

const char kUpdateApplicationWithBindOp[] =
    "UPDATE applications SET manifest = ?, path = ?,"
    "install_time = ? WHERE id = ?";

const char kDeleteApplicationWithBindOp[] =
    "DELETE FROM applications WHERE id = ?";

const char kSetManifestWithBindOp[] =
    "UPDATE applications SET manifest = ? WHERE id = ?";

const char kSetInstallTimeWithBindOp[] =
    "UPDATE applications SET install_time = ? WHERE id = ?";

const char kSetApplicationPathWithBindOp[] =
    "UPDATE applications SET path = ? WHERE id = ?";

const char kInsertEventsWithBindOp[] =
    "INSERT INTO registered_events (event_names, id) "
    "VALUES(?,?)";

const char kUpdateEventsWithBindOp[] =
    "UPDATE registered_events SET event_names = ? WHERE id = ?";

const char kDeleteEventsWithBindOp[] =
    "DELETE FROM registered_events WHERE id = ?";

}  // namespace db_store_constants
}  // namespace xwalk
