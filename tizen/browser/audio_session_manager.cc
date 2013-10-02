// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/audio_session_manager.h"

#include "base/logging.h"
#include "xwalk/tizen/browser/browser_mediaplayer_manager.h"

namespace tizen {

AudioSessionManager::AudioSessionManager(
    BrowserMediaPlayerManager* manager,
    MediaPlayerID player_id,
    int process_id)
    : event_type_(ASM_EVENT_NONE),
      manager_(manager),
      handle_(-1),
      process_id_(process_id),
      player_id_(player_id) {
}

AudioSessionManager::~AudioSessionManager() {
  UnregisterAudioSessionManager();
}

bool AudioSessionManager::RegisterAudioSessionManager(
    ASM_sound_events_t event_type,
    ASM_sound_cb_t notify_callback,
    void* callback_data) {
  int error = 0;
  event_type_ = event_type;

  if (!ASM_register_sound(process_id_, &handle_,
      event_type_, ASM_STATE_NONE,
      notify_callback, callback_data,
      ASM_RESOURCE_NONE, &error)) {
    LOG(ERROR) << "Register audio session manager failed. errcode=" << error;
    return false;
  }

  return true;
}

bool AudioSessionManager::UnregisterAudioSessionManager() {
  int error = 0;
  if (!ASM_unregister_sound(handle_, event_type_, &error)) {
    LOG(ERROR) << "Unregister audio session manager failed. errcode=" << error;
    return false;
  }

  return true;
}

bool AudioSessionManager::SetSoundState(ASM_sound_states_t state) {
  int error = 0;
  if (!ASM_set_sound_state(handle_, event_type_, state,
      ASM_RESOURCE_NONE, &error)) {
    LOG(ERROR) << "Set sound state =" << state << "failed. errcode=" << error;
    return false;
  }

  return true;
}

}  // namespace tizen
