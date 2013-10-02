// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/media/tizen_webmediaplayer_proxy.h"

#include "base/bind.h"
#include "base/message_loop.h"
#include "webkit/renderer/media/webmediaplayer_impl.h"
#include "xwalk/runtime/common/media/tizen_media_player_messages.h"
#include "xwalk/runtime/renderer/media/tizen_webmediaplayer_manager.h"

namespace webkit_media {

TizenWebMediaPlayerProxy::TizenWebMediaPlayerProxy(
    content::RenderView* render_view,
    TizenWebMediaPlayerManager* manager)
    : content::RenderViewObserver(render_view), manager_(manager) {
}

TizenWebMediaPlayerProxy::~TizenWebMediaPlayerProxy() {
  Send(new MediaPlayerHostMsg_DestroyAllMediaPlayers(routing_id()));
}

bool TizenWebMediaPlayerProxy::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(TizenWebMediaPlayerProxy, msg)
    IPC_MESSAGE_HANDLER(MediaPlayerMsg_DidMediaPlayerPlay, OnPlayerPlay)
    IPC_MESSAGE_HANDLER(MediaPlayerMsg_DidMediaPlayerPause, OnPlayerPause)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void TizenWebMediaPlayerProxy::Initialize(
    int player_id,
    int process_id,
    const GURL& url) {
  Send(new MediaPlayerHostMsg_MediaPlayerInitialize(
      routing_id(), player_id, process_id, url));
}

void TizenWebMediaPlayerProxy::Start(int player_id) {
  Send(new MediaPlayerHostMsg_MediaPlayerStart(routing_id(), player_id));
}

void TizenWebMediaPlayerProxy::Pause(int player_id) {
  Send(new MediaPlayerHostMsg_MediaPlayerPause(routing_id(), player_id));
}

void TizenWebMediaPlayerProxy::DestroyPlayer(int player_id) {
  Send(new MediaPlayerHostMsg_DestroyMediaPlayer(routing_id(), player_id));
}

WebMediaPlayerImpl* TizenWebMediaPlayerProxy::GetWebMediaPlayer(
    int player_id) {
  return static_cast<WebMediaPlayerImpl*>(
      manager_->GetMediaPlayer(player_id));
}

void TizenWebMediaPlayerProxy::OnPlayerPlay(int player_id) {
  WebMediaPlayerImpl* player = GetWebMediaPlayer(player_id);
  if (player)
    player->play();
}

void TizenWebMediaPlayerProxy::OnPlayerPause(int player_id) {
  WebMediaPlayerImpl* player = GetWebMediaPlayer(player_id);
  if (player)
    player->pause();
}

}  // namespace webkit_media
