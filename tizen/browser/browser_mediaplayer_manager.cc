// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/browser_mediaplayer_manager.h"

#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "xwalk/tizen/common/media_player_messages.h"

namespace tizen {

BrowserMediaPlayerManager::BrowserMediaPlayerManager(
    content::RenderViewHost* render_view_host)
    : WebContentsObserver(content::WebContents::FromRenderViewHost(
          render_view_host)) {
  resource_manager_.reset(new MurphyResourceManager(this));
}

BrowserMediaPlayerManager::~BrowserMediaPlayerManager() {}

BrowserMediaPlayerManager* BrowserMediaPlayerManager::Create(
    content::RenderViewHost* render_view_host) {
  return new BrowserMediaPlayerManager(render_view_host);
}

bool BrowserMediaPlayerManager::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BrowserMediaPlayerManager, msg)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_MediaPlayerInitialize, OnInitialize)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_MediaPlayerStarted, OnStart)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_MediaPlayerPaused, OnPause)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_DestroyMediaPlayer, OnDestroyPlayer)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_DestroyAllMediaPlayers,
                        OnDestroyAllMediaPlayers)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

MurphyResource* BrowserMediaPlayerManager::GetMurphyResource(
    MediaPlayerID player_id) {
  for (ScopedVector<MurphyResource>::iterator it =
      murphy_resources_.begin(); it != murphy_resources_.end();
      ++it) {
    if ((*it)->player_id() == player_id)
      return *it;
  }

  return NULL;
}

void BrowserMediaPlayerManager::ResourceNotifyCallback(
    mrp_res_resource_state_t state,
    MediaPlayerID player_id) {

  MurphyResource* resource = GetMurphyResource(player_id);

  mrp_res_resource_state_t prev_state = resource->GetResourceState();

  // Received a resource event from Murphy
  switch (state) {
    case MRP_RES_RESOURCE_AVAILABLE:
      if (prev_state == MRP_RES_RESOURCE_ACQUIRED)
        Send(new MediaPlayerMsg_MediaPlayerPause(routing_id(), player_id));
    case MRP_RES_RESOURCE_LOST:
      if (prev_state == MRP_RES_RESOURCE_ACQUIRED ||
          prev_state == MRP_RES_RESOURCE_LOST)
        Send(new MediaPlayerMsg_MediaPlayerPause(routing_id(), player_id));
      break;
    case MRP_RES_RESOURCE_ACQUIRED:
      if (prev_state == MRP_RES_RESOURCE_LOST)
        Send(new MediaPlayerMsg_MediaPlayerPlay(routing_id(), player_id));
      break;
    case MRP_RES_RESOURCE_PENDING:
    default:
      break;
    }
}

void BrowserMediaPlayerManager::OnInitialize(
    MediaPlayerID player_id,
    int process_id,
    const GURL& url) {

  // Create murphy resource for the given player id.
  if (resource_manager_ && resource_manager_->IsConnected()) {
    MurphyResource* resource = new MurphyResource(this,
        player_id, resource_manager_.get());
    RemoveMurphyResource(player_id);
    AddMurphyResource(resource);
  }
}

void BrowserMediaPlayerManager::OnDestroyAllMediaPlayers() {
  murphy_resources_.clear();
}

void BrowserMediaPlayerManager::OnDestroyPlayer(MediaPlayerID player_id) {
  RemoveMurphyResource(player_id);
}

void BrowserMediaPlayerManager::OnPause(MediaPlayerID player_id) {
  if (MurphyResource* resource = GetMurphyResource(player_id))
    resource->ReleaseResource();
}

void BrowserMediaPlayerManager::OnStart(MediaPlayerID player_id) {
  if (MurphyResource* resource = GetMurphyResource(player_id))
    resource->AcquireResource();
}

void BrowserMediaPlayerManager::AddMurphyResource(
    MurphyResource* resource) {
  DCHECK(!GetMurphyResource(resource->player_id()));
  murphy_resources_.push_back(resource);
}

void BrowserMediaPlayerManager::RemoveMurphyResource(
    MediaPlayerID player_id) {
  for (ScopedVector<MurphyResource>::iterator it =
      murphy_resources_.begin(); it != murphy_resources_.end();
      ++it) {
    if ((*it)->player_id() == player_id) {
      murphy_resources_.erase(it);
      break;
    }
  }
}

}  // namespace tizen
