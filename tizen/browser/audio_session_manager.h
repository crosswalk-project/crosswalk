// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_AUDIO_SESSION_MANAGER_H_
#define XWALK_TIZEN_BROWSER_AUDIO_SESSION_MANAGER_H_

#include <audio-session-manager.h>
#include "base/basictypes.h"

namespace tizen {
class BrowserMediaPlayerManager;

typedef int MediaPlayerID;

// This class manages communication between MediaPlayer and Audio
// Session Manager. It also defines Media Session policy and update
// the sound state to ASM server.
class AudioSessionManager {
 public:
  AudioSessionManager(
      BrowserMediaPlayerManager* manager,
      MediaPlayerID player_id,
      int process_id);
  ~AudioSessionManager();

  //  Register to ASM server
  bool RegisterAudioSessionManager(ASM_sound_events_t, ASM_sound_cb_t, void*);
  //  Unregister to ASM server
  bool UnregisterAudioSessionManager();

  bool SetSoundState(ASM_sound_states_t);
  MediaPlayerID player_id() const { return player_id_; }

  BrowserMediaPlayerManager* media_player_manager() {
    return manager_;
  }

 private:
  ASM_sound_events_t event_type_;
  BrowserMediaPlayerManager*  manager_;
  int handle_;
  int process_id_;
  MediaPlayerID player_id_;

  DISALLOW_COPY_AND_ASSIGN(AudioSessionManager);
};

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_AUDIO_SESSION_MANAGER_H_
