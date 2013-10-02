// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_MEDIA_TIZEN_AUDIO_SESSION_MANAGER_H_
#define XWALK_RUNTIME_BROWSER_MEDIA_TIZEN_AUDIO_SESSION_MANAGER_H_

#include "base/basictypes.h"

#include <audio-session-manager.h>

namespace content {
class TizenMediaPlayerManagerImpl;

// This class manages communication between MediaPlayer and Audio
// Session Manager. It also defines Media Session policy and update
// the sound state to ASM server.
class TizenAudioSessionManager {
 public:
  TizenAudioSessionManager(
      TizenMediaPlayerManagerImpl* manager,
      int player_id,
      int process_id);
  ~TizenAudioSessionManager();

  //  Register to ASM server
  bool RegisterAudioSessionManager(ASM_sound_events_t, ASM_sound_cb_t, void*);
  //  Unregister to ASM server
  bool UnregisterAudioSessionManager();

  bool SetSoundState(ASM_sound_states_t);
  int player_id() { return player_id_; }

  TizenMediaPlayerManagerImpl* media_player_manager() {
    return manager_;
  }

 private:
  ASM_sound_events_t event_type_;
  int handle_;

  TizenMediaPlayerManagerImpl*  manager_;
  int process_id_;
  int player_id_;

  DISALLOW_COPY_AND_ASSIGN(TizenAudioSessionManager);
};

} // namespace content

#endif // XWALK_RUNTIME_BROWSER_MEDIA_TIZEN_AUDIO_SESSION_MANAGER_H_
