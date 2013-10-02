// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_RENDERER_MEDIA_TIZEN_WEBMEDIAPLAYER_MANAGER_H_
#define XWALK_RUNTIME_RENDERER_MEDIA_TIZEN_WEBMEDIAPLAYER_MANAGER_H_

#include <map>

#include "base/basictypes.h"

namespace webkit_media {
class WebMediaPlayerImpl;

// Class for managing all the WebMediaPlayerImpl objects in the same
// RenderView.
class TizenWebMediaPlayerManager {
 public:
  TizenWebMediaPlayerManager();
  virtual ~TizenWebMediaPlayerManager();

  // Register and unregister a WebMediaPlayerImpl object.
  int RegisterMediaPlayer(WebMediaPlayerImpl* player);
  void UnregisterMediaPlayer(int player_id);

  // Get the pointer to WebMediaPlayerImpl given the |player_id|.
  WebMediaPlayerImpl* GetMediaPlayer(int player_id);

 private:
  // Info for all available WebMediaPlayerImpl on a page; kept so that
  // we can enumerate them to send updates.
  std::map<int, WebMediaPlayerImpl*> media_players_;

  int next_media_player_id_;

  DISALLOW_COPY_AND_ASSIGN(TizenWebMediaPlayerManager);
};

}  // namespace webkit_media

#endif  // XWALK_RUNTIME_RENDERER_MEDIA_TIZEN_WEBMEDIAPLAYER_MANAGER_H_
