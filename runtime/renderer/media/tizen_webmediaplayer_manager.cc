// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/media/tizen_webmediaplayer_manager.h"

#include "webkit/renderer/media/webmediaplayer_impl.h"

namespace webkit_media {

TizenWebMediaPlayerManager::TizenWebMediaPlayerManager()
    : next_media_player_id_(0) {
}

TizenWebMediaPlayerManager::~TizenWebMediaPlayerManager() {
  std::map<int, WebMediaPlayerImpl*>::iterator player_it;
  for (player_it = media_players_.begin();
      player_it != media_players_.end(); ++player_it) {
    WebMediaPlayerImpl* player = player_it->second;
    player->Detach();
  }
}

int TizenWebMediaPlayerManager::RegisterMediaPlayer(
    WebMediaPlayerImpl* player) {
  media_players_[next_media_player_id_] = player;

  return next_media_player_id_++;
}

void TizenWebMediaPlayerManager::UnregisterMediaPlayer(int player_id) {
  media_players_.erase(player_id);
}

WebMediaPlayerImpl* TizenWebMediaPlayerManager::GetMediaPlayer(
    int player_id) {
  std::map<int, WebMediaPlayerImpl*>::iterator iter =
      media_players_.find(player_id);
  if (iter != media_players_.end())
    return iter->second;

  return NULL;
}

}  // namespace webkit_media
