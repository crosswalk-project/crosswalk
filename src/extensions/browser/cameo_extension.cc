// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/extensions/browser/cameo_extension.h"

namespace cameo {
namespace extensions {

CameoExtension::CameoExtension(CameoExtension::Poster* poster,
                               const std::string& name,
                               CameoExtension::HandlerThread thread)
    : poster_(poster), name_(name), thread_(thread) {}

CameoExtension::~CameoExtension() {}

void CameoExtension::PostMessage(const int32_t render_view_id,
                                 const std::string& msg) {
  poster_->PostMessage(render_view_id, name_, msg);
}

}  // namespace extensions
}  // namespace cameo
