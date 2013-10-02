// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/media/tizen_media_player_manager_impl.h"

#include "content/public/browser/render_view_host.h"
#include "xwalk/runtime/common/media/tizen_media_player_messages.h"

namespace content {

TizenMediaPlayerManagerImpl::TizenMediaPlayerManagerImpl(
    RenderViewHost* render_view_host)
    : RenderViewHostObserver(render_view_host) {
}

TizenMediaPlayerManagerImpl::~TizenMediaPlayerManagerImpl() {}

bool TizenMediaPlayerManagerImpl::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(TizenMediaPlayerManagerImpl, msg)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_MediaPlayerInitialize, OnInitialize)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_MediaPlayerStart, OnStart)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_MediaPlayerPause, OnPause)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_DestroyMediaPlayer, OnDestroyPlayer)
    IPC_MESSAGE_HANDLER(MediaPlayerHostMsg_DestroyAllMediaPlayers,
                        OnDestroyAllMediaPlayers)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

TizenAudioSessionManager* TizenMediaPlayerManagerImpl::GetAudioSessionManager(
    int player_id) {
  for (ScopedVector<TizenAudioSessionManager>::iterator it =
      audio_session_managers_.begin(); it != audio_session_managers_.end();
      ++it) {
    if ((*it)->player_id() == player_id)
      return *it;
  }

  return NULL;
}

ASM_cb_result_t TizenMediaPlayerManagerImpl::AudioSessionEventPause(
    ASM_event_sources_t event_source,
    int player_id)
{
  switch (event_source) {
    case ASM_EVENT_SOURCE_CALL_START:
    case ASM_EVENT_SOURCE_ALARM_START:
    case ASM_EVENT_SOURCE_MEDIA:
    case ASM_EVENT_SOURCE_EMERGENCY_START:
    case ASM_EVENT_SOURCE_OTHER_PLAYER_APP:
    case ASM_EVENT_SOURCE_RESOURCE_CONFLICT:
      Send(new MediaPlayerMsg_DidMediaPlayerPause(routing_id(), player_id));
      return ASM_CB_RES_PAUSE;
    default:
      return ASM_CB_RES_NONE;
    }
}

ASM_cb_result_t TizenMediaPlayerManagerImpl::AudioSessionEventPlay(
    ASM_event_sources_t event_source,
    int player_id) {
  switch (event_source) {
    case ASM_EVENT_SOURCE_ALARM_END:
      Send(new MediaPlayerMsg_DidMediaPlayerPlay(routing_id(), player_id));
      return ASM_CB_RES_PLAYING;
    default:
      return ASM_CB_RES_NONE;
  }
}

static ASM_cb_result_t AudioSessionNotifyCallback(
    int handle,
    ASM_event_sources_t event_source,
    ASM_sound_commands_t command,
    unsigned int sound_status,
    void* callback_data) {
  TizenAudioSessionManager* data =
      static_cast<TizenAudioSessionManager*>(callback_data);

  TizenMediaPlayerManagerImpl* manager = data->media_player_manager();
  if (command == ASM_COMMAND_STOP || command == ASM_COMMAND_PAUSE)
    return manager->AudioSessionEventPause(event_source, data->player_id());
  if (command == ASM_COMMAND_PLAY || command == ASM_COMMAND_RESUME)
    return manager->AudioSessionEventPlay(event_source, data->player_id());

  return ASM_CB_RES_NONE;
}

void TizenMediaPlayerManagerImpl::OnInitialize(
    int player_id,
    int process_id,
    const GURL& url) {
  RemoveAudioSessionManager(player_id);
  TizenAudioSessionManager* session_manager =
      new TizenAudioSessionManager(this, player_id, process_id);

  session_manager->RegisterAudioSessionManager(
      ASM_EVENT_SHARE_MMPLAYER,
      AudioSessionNotifyCallback,
      session_manager);
  session_manager->SetSoundState(ASM_STATE_PAUSE);

  AddAudioSessionManager(session_manager);
}

void TizenMediaPlayerManagerImpl::OnDestroyAllMediaPlayers() {
  audio_session_managers_.clear();
}

void TizenMediaPlayerManagerImpl::OnDestroyPlayer(int player_id) {
  RemoveAudioSessionManager(player_id);
}

void TizenMediaPlayerManagerImpl::OnPause(int player_id) {
  TizenAudioSessionManager* session_manager = GetAudioSessionManager(player_id);
  if (session_manager)
    session_manager->SetSoundState(ASM_STATE_PAUSE);
}

void TizenMediaPlayerManagerImpl::OnStart(int player_id) {
  TizenAudioSessionManager* session_manager = GetAudioSessionManager(player_id);
  if (session_manager)
    session_manager->SetSoundState(ASM_STATE_PLAYING);
}

void TizenMediaPlayerManagerImpl::AddAudioSessionManager(
    TizenAudioSessionManager* session_manager) {
  DCHECK(!GetAudioSessionManager(session_manager->player_id()));
  audio_session_managers_.push_back(session_manager);
}

void TizenMediaPlayerManagerImpl::RemoveAudioSessionManager(int player_id) {
  for (ScopedVector<TizenAudioSessionManager>::iterator it =
      audio_session_managers_.begin(); it != audio_session_managers_.end();
      ++it) {
    if ((*it)->player_id() == player_id) {
      audio_session_managers_.erase(it);
      break;
    }
  }
}

}  // namespace content
