// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/murphy_resource_manager.h"

#include "base/logging.h"
#include "xwalk/tizen/browser/browser_mediaplayer_manager.h"

namespace {

static void NotifyCallback(mrp_res_context_t* context,
    mrp_res_error_t error, void* callback_data) {
  if (error != MRP_RES_ERROR_NONE) {
    LOG(ERROR) << "Error message received from Murphy. errcode=" << error;
    return;
  }

  if (context->state == MRP_RES_DISCONNECTED) {
    tizen::MurphyResourceManager* resource =
      static_cast<tizen::MurphyResourceManager*> (callback_data);
    if (resource)
      resource->DisconnectFromMurphy();
  }
}

}  // namespace

namespace tizen {

MurphyResourceManager::MurphyResourceManager(BrowserMediaPlayerManager* manager)
    : manager_(manager),
      murphy_context_(NULL),
      murphy_mainloop_(new MurphyMainloop()),
      mainloop_(murphy_mainloop_->getMainloop()) {
  // Connect to murphy.
  ConnectToMurphy();
}

MurphyResourceManager::~MurphyResourceManager() {
  DisconnectFromMurphy();
}

void MurphyResourceManager::ConnectToMurphy() {
  if (murphy_context_) {
    mrp_res_destroy(murphy_context_);
    murphy_context_ = NULL;
  }

  if (!murphy_context_)
    murphy_context_ = mrp_res_create(mainloop_, NotifyCallback, this);
}

bool MurphyResourceManager::IsConnected() const {
  if (!murphy_context_)
    return false;

  return murphy_context_->state == MRP_RES_CONNECTED;
}

void MurphyResourceManager::DisconnectFromMurphy() {
  if (!murphy_context_)
    return;

  mrp_res_destroy(murphy_context_);
  murphy_context_ = NULL;
}

mrp_res_context_t* MurphyResourceManager::GetContext() {
  return murphy_context_;
}

}  // namespace tizen
