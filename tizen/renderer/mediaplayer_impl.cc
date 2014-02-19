// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/renderer/mediaplayer_impl.h"

#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/platform/WebURL.h"

namespace tizen {

MediaPlayerImpl::MediaPlayerImpl(
    content::RenderView* render_view,
    blink::WebFrame* frame,
    blink::WebMediaPlayerClient* client,
    base::WeakPtr<content::WebMediaPlayerDelegate> delegate,
    RendererMediaPlayerManager* manager,
    const content::WebMediaPlayerParams& params)
    : WebMediaPlayerImpl(render_view, frame, client, delegate, params),
      manager_(manager) {
  DCHECK(manager_);

  player_id_ = manager_->RegisterMediaPlayer(this);
}

MediaPlayerImpl::~MediaPlayerImpl() {
  if (manager_) {
    manager_->DestroyPlayer(player_id_);
    manager_->UnregisterMediaPlayer(player_id_);
  }
}

void MediaPlayerImpl::WillDestroyCurrentMessageLoop() {
  if (manager_)
    manager_->UnregisterMediaPlayer(player_id_);
  Detach();
}

void MediaPlayerImpl::Detach() {
  manager_ = NULL;
}

void MediaPlayerImpl::load(LoadType load_type, const blink::WebURL& url,
                           CORSMode cors_mode) {
  InitializeMediaPlayer(url);
  WebMediaPlayerImpl::load(load_type, url, cors_mode);
}

void MediaPlayerImpl::InitializeMediaPlayer(const blink::WebURL& url) {
  if (manager_)
    manager_->Initialize(player_id_, getpid(), url);
}

void MediaPlayerImpl::play() {
  if (manager_)
    manager_->Start(player_id_);
  WebMediaPlayerImpl::play();
}

void MediaPlayerImpl::pause() {
  if (manager_)
    manager_->Pause(player_id_);
  WebMediaPlayerImpl::pause();
}

}  // namespace tizen
