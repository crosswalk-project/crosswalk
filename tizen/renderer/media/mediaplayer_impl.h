// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef XWALK_TIZEN_RENDERER_MEDIA_MEDIAPLAYER_IMPL_H_
#define XWALK_TIZEN_RENDERER_MEDIA_MEDIAPLAYER_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "media/blink/webmediaplayer_impl.h"
#include "xwalk/tizen/renderer/media/renderer_mediaplayer_manager.h"

namespace tizen {

// Substitute for WebMediaPlayerImpl to be used in Tizen.
class MediaPlayerImpl : public media::WebMediaPlayerImpl {
 public:
  MediaPlayerImpl(
      blink::WebLocalFrame* frame,
      blink::WebMediaPlayerClient* client,
      base::WeakPtr<media::WebMediaPlayerDelegate> delegate,
      RendererMediaPlayerManager* manager,
      const media::WebMediaPlayerParams& params);
  virtual ~MediaPlayerImpl();

  // WebMediaPlayerImpl method.
  void load(LoadType load_type,
            const blink::WebURL& url,
            CORSMode cors_mode) override;

  // Playback controls.
  virtual void play();
  virtual void pause();

  // Detach the player from its manager.
  void Detach();

  // Functions called when media player status changes.
  void OnMediaPlayerPlay();
  void OnMediaPlayerPause();

 private:
  void InitializeMediaPlayer(const blink::WebURL& url);

  blink::WebMediaPlayerClient* client_;

  // Manager for managing this object and for delegating method calls on
  // Render Thread.
  RendererMediaPlayerManager* manager_;

  // Player ID assigned by the |manager_|.
  MediaPlayerID player_id_;

  DISALLOW_COPY_AND_ASSIGN(MediaPlayerImpl);
};

}  // namespace tizen

#endif  // XWALK_TIZEN_RENDERER_MEDIA_MEDIAPLAYER_IMPL_H_
