// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_MEDIA_MEDIA_WEBCONTENTS_OBSERVER_H_
#define XWALK_TIZEN_BROWSER_MEDIA_MEDIA_WEBCONTENTS_OBSERVER_H_

#include "base/compiler_specific.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "content/common/content_export.h"
#include "content/public/browser/web_contents_observer.h"
#include "xwalk/tizen/browser/media/murphy_resource_manager.h"

namespace tizen {

class BrowserMediaPlayerManager;
class RenderViewHost;

// This class manages all RenderFrame based media related managers at the
// browser side. It receives IPC messages from media RenderFrameObservers and
// forwards them to the corresponding managers. The managers are responsible
// for sending IPCs back to the RenderFrameObservers at the render side.
class CONTENT_EXPORT MediaWebContentsObserver :
    public content::WebContentsObserver {
 public:
  explicit MediaWebContentsObserver(content::RenderViewHost* render_view_host);
  virtual ~MediaWebContentsObserver();

  // WebContentsObserver implementations.
  void RenderFrameDeleted(
      content::RenderFrameHost* render_frame_host) override;
  bool OnMessageReceived(const IPC::Message& message,
      content::RenderFrameHost* render_frame_host) override;

  // Gets the media player manager associated with |render_frame_host|. Creates
  // a new one if it doesn't exist. The caller doesn't own the returned pointer.
  BrowserMediaPlayerManager* GetMediaPlayerManager(
      content::RenderFrameHost* render_frame_host);

 private:
  // Map from RenderFrameHost* to BrowserMediaPlayerManager.
  typedef base::ScopedPtrHashMap<uintptr_t, BrowserMediaPlayerManager>
      MediaPlayerManagerMap;
  MediaPlayerManagerMap media_player_managers_;

  scoped_ptr<MurphyResourceManager> resource_manager_;

  DISALLOW_COPY_AND_ASSIGN(MediaWebContentsObserver);
};

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_MEDIA_MEDIA_WEBCONTENTS_OBSERVER_H_
