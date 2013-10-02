// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_MEDIA_TIZEN_WEBMEDIAPLAYER_PROXY_H_
#define XWALK_RUNTIME_RENDERER_MEDIA_TIZEN_WEBMEDIAPLAYER_PROXY_H_

#include <string>

#include "base/time.h"
#include "content/public/renderer/render_view_observer.h"
#include "googleurl/src/gurl.h"

namespace webkit_media {
class TizenWebMediaPlayerManager;
class WebMediaPlayerImpl;

// This class manages IPC communication between WebMediaPlayerImpl and the
// TizenMediaPlayerManagerImpl in the browser process.
class TizenWebMediaPlayerProxy : public content::RenderViewObserver {
 public:
  // Construct a TizenWebMediaPlayerProxy object for the |render_view|.
  // |manager| is passed to this class so that it can find the right
  // WebMediaPlayerImpl using player IDs.
  TizenWebMediaPlayerProxy(
      content::RenderView* render_view,
      TizenWebMediaPlayerManager* manager);
  virtual ~TizenWebMediaPlayerProxy();

  // RenderViewObserver overrides.
  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE;

  // Initializes a TizenMediaPlayerManagerImpl object in browser process.
  void Initialize(int player_id,
                  int procedd_id,
                  const GURL& url);

  // Starts the player.
  void Start(int player_id);

  // Pauses the player.
  void Pause(int player_id);

  // Destroy the player in the browser process
  void DestroyPlayer(int player_id);

 private:
  WebMediaPlayerImpl* GetWebMediaPlayer(int player_id);

  void OnPlayerPlay(int player_id);
  void OnPlayerPause(int player_id);

  TizenWebMediaPlayerManager* manager_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(TizenWebMediaPlayerProxy);
};

}  // namespace webkit_media

#endif  // XWALK_RUNTIME_RENDERER_MEDIA_TIZEN_WEBMEDIAPLAYER_PROXY_H_
