// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/browser_mediaplayer_manager.h"

#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "xwalk/tizen/browser/audio_session_manager_init.h"
#include "xwalk/tizen/common/media_player_messages.h"

namespace tizen {

BrowserMediaPlayerManager::BrowserMediaPlayerManager(
    content::RenderViewHost* render_view_host)
    : WebContentsObserver(content::WebContents::FromRenderViewHost(
          render_view_host)) {
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

AudioSessionManager* BrowserMediaPlayerManager::GetAudioSessionManager(
    MediaPlayerID player_id) {
  for (ScopedVector<AudioSessionManager>::iterator it =
      audio_session_managers_.begin(); it != audio_session_managers_.end();
      ++it) {
    if ((*it)->player_id() == player_id)
      return *it;
  }

  return NULL;
}

ASM_cb_result_t BrowserMediaPlayerManager::AudioSessionEventPause(
    ASM_event_sources_t event_source,
    MediaPlayerID player_id) {
  switch (event_source) {
    case ASM_EVENT_SOURCE_CALL_START:
    case ASM_EVENT_SOURCE_ALARM_START:
    case ASM_EVENT_SOURCE_MEDIA:
    case ASM_EVENT_SOURCE_EMERGENCY_START:
    case ASM_EVENT_SOURCE_OTHER_PLAYER_APP:
    case ASM_EVENT_SOURCE_RESOURCE_CONFLICT:
      Send(new MediaPlayerMsg_MediaPlayerPause(routing_id(), player_id));
      return ASM_CB_RES_PAUSE;
    default:
      return ASM_CB_RES_NONE;
    }
}

ASM_cb_result_t BrowserMediaPlayerManager::AudioSessionEventPlay(
    ASM_event_sources_t event_source,
    MediaPlayerID player_id) {
  switch (event_source) {
    case ASM_EVENT_SOURCE_ALARM_END:
      Send(new MediaPlayerMsg_MediaPlayerPlay(routing_id(), player_id));
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
  AudioSessionManager* data =
      static_cast<AudioSessionManager*>(callback_data);

  BrowserMediaPlayerManager* manager = data->media_player_manager();
  if (command == ASM_COMMAND_STOP || command == ASM_COMMAND_PAUSE)
    return manager->AudioSessionEventPause(event_source, data->player_id());
  if (command == ASM_COMMAND_PLAY || command == ASM_COMMAND_RESUME)
    return manager->AudioSessionEventPlay(event_source, data->player_id());

  return ASM_CB_RES_NONE;
}

void BrowserMediaPlayerManager::OnInitialize(
    MediaPlayerID player_id,
    int process_id,
    const GURL& url) {

  // Initialize the audio session manager library.
  if (!InitializeAudioSessionManager()) {
    DLOG(WARNING) << "Failed on loading the audio session manager library";
    return;
  }

  RemoveAudioSessionManager(player_id);
  AudioSessionManager* session_manager =
      new AudioSessionManager(this, player_id, process_id);

  session_manager->RegisterAudioSessionManager(
      ASM_EVENT_SHARE_MMPLAYER,
      AudioSessionNotifyCallback,
      session_manager);
  session_manager->SetSoundState(ASM_STATE_PAUSE);

  AddAudioSessionManager(session_manager);
}

void BrowserMediaPlayerManager::OnDestroyAllMediaPlayers() {
  audio_session_managers_.clear();
}

void BrowserMediaPlayerManager::OnDestroyPlayer(MediaPlayerID player_id) {
  RemoveAudioSessionManager(player_id);
}

void BrowserMediaPlayerManager::OnPause(MediaPlayerID player_id) {
  AudioSessionManager* session_manager = GetAudioSessionManager(player_id);
  if (session_manager)
    session_manager->SetSoundState(ASM_STATE_PAUSE);
}

void BrowserMediaPlayerManager::OnStart(MediaPlayerID player_id) {
  AudioSessionManager* session_manager = GetAudioSessionManager(player_id);
  if (session_manager)
    session_manager->SetSoundState(ASM_STATE_PLAYING);
}

void BrowserMediaPlayerManager::AddAudioSessionManager(
    AudioSessionManager* session_manager) {
  DCHECK(!GetAudioSessionManager(session_manager->player_id()));
  audio_session_managers_.push_back(session_manager);
}

void BrowserMediaPlayerManager::RemoveAudioSessionManager(
    MediaPlayerID player_id) {
  for (ScopedVector<AudioSessionManager>::iterator it =
      audio_session_managers_.begin(); it != audio_session_managers_.end();
      ++it) {
    if ((*it)->player_id() == player_id) {
      audio_session_managers_.erase(it);
      break;
    }
  }
}

}  // namespace tizen
