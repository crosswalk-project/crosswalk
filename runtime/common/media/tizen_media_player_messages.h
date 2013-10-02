// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// IPC messages for tizen media player.
// Multiply-included message file, hence no include guard.

#include <string>

#include "content/common/content_export.h"
#include "googleurl/src/gurl.h"
#include "ipc/ipc_message_macros.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT CONTENT_EXPORT
#define IPC_MESSAGE_START MediaPlayerMsgStart

// Messages for notifying the render process of media playback status -------

// The player started playing.
IPC_MESSAGE_ROUTED1(MediaPlayerMsg_DidMediaPlayerPlay,
                    int /* player_id */)

// The player was paused.
IPC_MESSAGE_ROUTED1(MediaPlayerMsg_DidMediaPlayerPause,
                    int /* player_id */)

// Messages for controllering the media playback in browser process ----------

// Destroy the media player object.
IPC_MESSAGE_ROUTED1(MediaPlayerHostMsg_DestroyMediaPlayer,
                    int /* player_id */)

// Destroy all the players.
IPC_MESSAGE_ROUTED0(MediaPlayerHostMsg_DestroyAllMediaPlayers)

// Initialize a media player object with the given player_id.
IPC_MESSAGE_ROUTED3(
    MediaPlayerHostMsg_MediaPlayerInitialize,
    int /* player_id */,
    int /* process_id */,
    GURL /* url */)

// Pause the player.
IPC_MESSAGE_ROUTED1(MediaPlayerHostMsg_MediaPlayerPause,
                    int /* player_id */)

// Start the player for playback.
IPC_MESSAGE_ROUTED1(MediaPlayerHostMsg_MediaPlayerStart,
                    int /* player_id */)
