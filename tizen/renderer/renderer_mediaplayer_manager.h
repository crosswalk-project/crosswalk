// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_RENDERER_RENDERER_MEDIAPLAYER_MANAGER_H_
#define XWALK_TIZEN_RENDERER_RENDERER_MEDIAPLAYER_MANAGER_H_

#include <map>

#include "base/basictypes.h"
#include "content/public/renderer/render_view_observer.h"
#include "url/gurl.h"

namespace tizen {
class MediaPlayerImpl;

typedef int MediaPlayerID;

// Class for managing all the MediaPlayerImpl objects in the same
// RenderView.
class RendererMediaPlayerManager : public content::RenderViewObserver {
 public:
  // Constructs a RendererMediaPlayerManager object for the |render_view|.
  explicit RendererMediaPlayerManager(content::RenderView* render_view);
  virtual ~RendererMediaPlayerManager();

  // RenderViewObserver overrides.
  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE;

  // Initializes a BrowserMediaPlayerManager object in browser process.
  void Initialize(MediaPlayerID player_id,
                  int procedd_id,
                  const GURL& url);

  // Starts the player.
  void Start(MediaPlayerID player_id);

  // Pausees the player.
  void Pause(MediaPlayerID player_id);

  // Destroy the player in the browser process
  void DestroyPlayer(MediaPlayerID player_id);

  // Register a MediaPlayerImpl object and return the ID of the player.
  MediaPlayerID RegisterMediaPlayer(MediaPlayerImpl* player);

  // Unregister a MediaPlayerImpl object of given |player_id|.
  void UnregisterMediaPlayer(MediaPlayerID player_id);

 private:
  // Get the pointer to MediaPlayerImpl of given |player_id|.
  MediaPlayerImpl* GetMediaPlayer(MediaPlayerID player_id);

  // Message handlers.
  void OnPlayerPlay(MediaPlayerID player_id);
  void OnPlayerPause(MediaPlayerID player_id);

  // Info for all available MediaPlayerImpl on a page; kept so that
  // we can enumerate them to send updates.
  std::map<MediaPlayerID, MediaPlayerImpl*> media_players_;

  MediaPlayerID next_media_player_id_;

  DISALLOW_COPY_AND_ASSIGN(RendererMediaPlayerManager);
};

}  // namespace tizen

#endif  // XWALK_TIZEN_RENDERER_RENDERER_MEDIAPLAYER_MANAGER_H_
