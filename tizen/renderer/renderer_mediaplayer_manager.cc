// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/renderer/renderer_mediaplayer_manager.h"

#include "xwalk/tizen/common/media_player_messages.h"
#include "xwalk/tizen/renderer/mediaplayer_impl.h"

namespace tizen {

RendererMediaPlayerManager::RendererMediaPlayerManager(
    content::RenderView* render_view)
    : content::RenderViewObserver(render_view),
      next_media_player_id_(0) {
}

RendererMediaPlayerManager::~RendererMediaPlayerManager() {
  std::map<MediaPlayerID, MediaPlayerImpl*>::iterator player_it;
  for (player_it = media_players_.begin();
      player_it != media_players_.end(); ++player_it) {
    MediaPlayerImpl* player = player_it->second;
    player->Detach();
  }

  Send(new MediaPlayerHostMsg_DestroyAllMediaPlayers(routing_id()));
}

bool RendererMediaPlayerManager::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RendererMediaPlayerManager, msg)
    IPC_MESSAGE_HANDLER(MediaPlayerMsg_MediaPlayerPlay, OnPlayerPlay)
    IPC_MESSAGE_HANDLER(MediaPlayerMsg_MediaPlayerPause, OnPlayerPause)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void RendererMediaPlayerManager::Initialize(
    MediaPlayerID player_id,
    int process_id,
    const GURL& url) {
  Send(new MediaPlayerHostMsg_MediaPlayerInitialize(
      routing_id(), player_id, process_id, url));
}

void RendererMediaPlayerManager::Start(MediaPlayerID player_id) {
  Send(new MediaPlayerHostMsg_MediaPlayerStarted(routing_id(), player_id));
}

void RendererMediaPlayerManager::Pause(MediaPlayerID player_id) {
  Send(new MediaPlayerHostMsg_MediaPlayerPaused(routing_id(), player_id));
}

void RendererMediaPlayerManager::DestroyPlayer(MediaPlayerID player_id) {
  Send(new MediaPlayerHostMsg_DestroyMediaPlayer(routing_id(), player_id));
}

MediaPlayerID RendererMediaPlayerManager::RegisterMediaPlayer(
    MediaPlayerImpl* player) {
  media_players_[next_media_player_id_] = player;

  return next_media_player_id_++;
}

void RendererMediaPlayerManager::UnregisterMediaPlayer(
    MediaPlayerID player_id) {
  media_players_.erase(player_id);
}

MediaPlayerImpl* RendererMediaPlayerManager::GetMediaPlayer(
    MediaPlayerID player_id) {
  std::map<MediaPlayerID, MediaPlayerImpl*>::iterator iter =
      media_players_.find(player_id);
  if (iter != media_players_.end())
    return iter->second;

  return NULL;
}

void RendererMediaPlayerManager::OnPlayerPlay(MediaPlayerID player_id) {
  if (MediaPlayerImpl* player = GetMediaPlayer(player_id))
    player->play();
}

void RendererMediaPlayerManager::OnPlayerPause(MediaPlayerID player_id) {
  if (MediaPlayerImpl* player = GetMediaPlayer(player_id))
    player->pause();
}

}  // namespace tizen
