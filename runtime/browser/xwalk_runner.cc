// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner.h"

#include "base/logging.h"

namespace xwalk {

namespace {

XWalkRunner* g_xwalk_runner = NULL;

}  // namespace

XWalkRunner::XWalkRunner() {
  VLOG(1) << "Creating XWalkRunner object.";
  DCHECK(!g_xwalk_runner);
  g_xwalk_runner = this;
}

XWalkRunner::~XWalkRunner() {
  DCHECK(g_xwalk_runner);
  g_xwalk_runner = NULL;
  VLOG(1) << "Destroying XWalkRunner object.";
}

// static
XWalkRunner* XWalkRunner::Get() {
  return g_xwalk_runner;
}

// static
scoped_ptr<XWalkRunner> XWalkRunner::Create() {
  return scoped_ptr<XWalkRunner>(new XWalkRunner);
}

}  // namespace xwalk
