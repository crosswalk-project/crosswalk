// Copyright (c) 2011 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/murphy_resource.h"

#include "xwalk/tizen/browser/browser_mediaplayer_manager.h"

namespace {

const char kMediaApplicationClass[] = "player";
const char kMediaStreamName[] = "audio_playback";
const char kMediaRole[] = "browser";

static void NotifyCallback(mrp_res_context_t* context,
    const mrp_res_resource_set_t* resource_set,
    void* callback_data) {
  tizen::MurphyResource* resource =
    static_cast<tizen::MurphyResource*> (callback_data);
  tizen::BrowserMediaPlayerManager* manager = resource->media_player_manager();
  if (!manager)
    return;

  manager->ResourceNotifyCallback(resource_set->state, resource->player_id());
  resource->SetResourceState(resource_set->state);
}

}  // namespace

namespace tizen {

MurphyResource::MurphyResource(
    BrowserMediaPlayerManager* manager,
    MediaPlayerID player_id,
    MurphyResourceManager* resource_manager)
    : manager_(manager),
      player_id_(player_id),
      resource_manager_(resource_manager),
      resource_state_(MRP_RES_RESOURCE_PENDING) {
  mrp_res_context_t* context = resource_manager_->GetContext();
  if (!context)
    return;

  resource_set_ = mrp_res_create_resource_set(context,
      kMediaApplicationClass, NotifyCallback, this);
  if (!resource_set_)
    return;

  mrp_res_resource_t* resource = mrp_res_create_resource(context,
      resource_set_, kMediaStreamName, true, true);
  mrp_res_attribute_t* attr = mrp_res_get_attribute_by_name(
      context, resource, "role");
  if (attr)
    mrp_res_set_attribute_string(context, attr, kMediaRole);

  mrp_res_release_resource_set(context, resource_set_);
}

void MurphyResource::AcquireResource() {
  if (!resource_manager_ || !resource_set_)
    return;

  // Call acquire
  mrp_res_acquire_resource_set(resource_manager_->GetContext(), resource_set_);
}

void MurphyResource::ReleaseResource() {
  if (!resource_manager_ || !resource_set_)
    return;

  // Call release
  mrp_res_release_resource_set(resource_manager_->GetContext(), resource_set_);
}

MurphyResource::~MurphyResource() {
  ReleaseResource();

  if (!resource_manager_ || !resource_set_)
    return;

  // Delete resource set (and the resource inside it)
  mrp_res_delete_resource_set(resource_manager_->GetContext(), resource_set_);
}

}  // namespace tizen
