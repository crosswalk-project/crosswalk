// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/tizen/browser/media/media_webcontents_observer.h"

#include "content/public/browser/web_contents.h"
#include "xwalk/tizen/browser/media/browser_mediaplayer_manager.h"
#include "xwalk/tizen/browser/media/murphy_resource_manager.h"
#include "xwalk/tizen/common/media/media_player_messages.h"

namespace tizen {

MediaWebContentsObserver::MediaWebContentsObserver(
    content::RenderViewHost* render_view_host)
    : WebContentsObserver(content::WebContents::FromRenderViewHost(
          render_view_host)) {
  resource_manager_.reset(new MurphyResourceManager());
}

MediaWebContentsObserver::~MediaWebContentsObserver() {}

void MediaWebContentsObserver::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  uintptr_t key = reinterpret_cast<uintptr_t>(render_frame_host);
  media_player_managers_.erase(key);
}

bool MediaWebContentsObserver::OnMessageReceived(
    const IPC::Message& msg,
    content::RenderFrameHost* render_frame_host) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(MediaWebContentsObserver, msg)
    IPC_MESSAGE_FORWARD(MediaPlayerHostMsg_MediaPlayerInitialize,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnInitialize)
    IPC_MESSAGE_FORWARD(MediaPlayerHostMsg_MediaPlayerStarted,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnStart)
    IPC_MESSAGE_FORWARD(MediaPlayerHostMsg_MediaPlayerPaused,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnPause)
    IPC_MESSAGE_FORWARD(MediaPlayerHostMsg_DestroyMediaPlayer,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnDestroyPlayer)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

BrowserMediaPlayerManager* MediaWebContentsObserver::GetMediaPlayerManager(
    content::RenderFrameHost* render_frame_host) {
  uintptr_t key = reinterpret_cast<uintptr_t>(render_frame_host);
  if (!media_player_managers_.contains(key)) {
    media_player_managers_.set(
        key,
        make_scoped_ptr(BrowserMediaPlayerManager::Create(render_frame_host,
            resource_manager_.get())));
  }

  return media_player_managers_.get(key);
}

}  // namespace tizen
