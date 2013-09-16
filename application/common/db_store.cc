// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/db_store.h"

namespace xwalk {
namespace application {

DBStore::DBStore(base::FilePath path) : data_path_(path) {
}

DBStore::~DBStore() {
}

}  // namespace application
}  // namespace xwalk

