// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/crosswalk.h"

#include "base/logging.h"

namespace xwalk {

namespace {

Crosswalk* g_crosswalk = NULL;

}  // namespace

Crosswalk::Crosswalk() {
  VLOG(1) << "Creating Crosswalk object.";
  DCHECK(!g_crosswalk);
  g_crosswalk = this;
}

Crosswalk::~Crosswalk() {
  DCHECK(g_crosswalk);
  g_crosswalk = NULL;
  VLOG(1) << "Destroying Crosswalk object.";
}

// static
Crosswalk* Crosswalk::Get() {
  return g_crosswalk;
}

// static
scoped_ptr<Crosswalk> Crosswalk::Create() {
  return scoped_ptr<Crosswalk>(new Crosswalk);
}

}  // namespace xwalk
