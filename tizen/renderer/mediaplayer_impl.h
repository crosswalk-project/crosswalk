// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef XWALK_TIZEN_RENDERER_MEDIAPLAYER_IMPL_H_
#define XWALK_TIZEN_RENDERER_MEDIAPLAYER_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/renderer/media/webmediaplayer_impl.h"
#include "xwalk/tizen/renderer/renderer_mediaplayer_manager.h"

namespace tizen {

// Substitute for WebMediaPlayerImpl to be used in Tizen.
class MediaPlayerImpl : public content::WebMediaPlayerImpl {
 public:
  MediaPlayerImpl(
      WebKit::WebFrame* frame,
      WebKit::WebMediaPlayerClient* client,
      base::WeakPtr<content::WebMediaPlayerDelegate> delegate,
      RendererMediaPlayerManager* manager,
      const content::WebMediaPlayerParams& params);
  virtual ~MediaPlayerImpl();

  // WebMediaPlayerImpl method.
  virtual void load(LoadType load_type,
                    const WebKit::WebURL& url,
                    CORSMode cors_mode) OVERRIDE;

  // Playback controls.
  virtual void play();
  virtual void pause();

  // As we are closing the app, |main_loop_| is destroyed even before
  // this object gets destructed, so we need to know when |main_loop_|
  // is being destroyed and we can stop posting playback controls.
  virtual void WillDestroyCurrentMessageLoop() OVERRIDE;

  // Detach the player from its manager.
  void Detach();

 private:
  void InitializeMediaPlayer(const WebKit::WebURL& url);

  // Manager for managing this object and for delegating method calls on
  // Render Thread.
  RendererMediaPlayerManager* manager_;

  // Player ID assigned by the |manager_|.
  MediaPlayerID player_id_;

  DISALLOW_COPY_AND_ASSIGN(MediaPlayerImpl);
};

}  // namespace tizen

#endif  // XWALK_TIZEN_RENDERER_MEDIAPLAYER_IMPL_H_
