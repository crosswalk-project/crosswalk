// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/crosswalk.h"

#include "base/logging.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

namespace xwalk {

namespace {

Crosswalk* g_crosswalk = NULL;

}  // namespace

Crosswalk::Crosswalk() {
  VLOG(0) << "Creating Crosswalk object.";
  DCHECK(!g_crosswalk);
  g_crosswalk = this;

  // Do this after setting g_crosswalk so Crosswalk::Get() is always reliable.
  content_browser_client_.reset(new XWalkContentBrowserClient);
}

Crosswalk::~Crosswalk() {
  DCHECK(g_crosswalk);
  g_crosswalk = NULL;
  VLOG(0) << "Destroying Crosswalk object.";
}

// static
Crosswalk* Crosswalk::Get() {
  return g_crosswalk;
}

void Crosswalk::PreMainMessageLoopRun() {
  runtime_context_.reset(new RuntimeContext);
}

void Crosswalk::PostMainMessageLoopRun() {
  runtime_context_.reset();
}

// static
scoped_ptr<Crosswalk> Crosswalk::Create() {
  return scoped_ptr<Crosswalk>(new Crosswalk);
}

content::ContentBrowserClient* Crosswalk::GetContentBrowserClient() {
  return content_browser_client_.get();
}

}  // namespace xwalk
