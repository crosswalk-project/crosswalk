// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_runner.h"

#include "base/logging.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

namespace xwalk {

namespace {

XWalkRunner* g_xwalk_runner = NULL;

}  // namespace

XWalkRunner::XWalkRunner() {
  VLOG(1) << "Creating XWalkRunner object.";
  DCHECK(!g_xwalk_runner);
  g_xwalk_runner = this;

  // Initializing after the g_xwalk_runner is set to ensure XWalkRunner::Get()
  // can be used in all sub objects if needed.
  content_browser_client_.reset(new XWalkContentBrowserClient);
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

content::ContentBrowserClient* XWalkRunner::GetContentBrowserClient() {
  return content_browser_client_.get();
}

}  // namespace xwalk
