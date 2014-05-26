// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_BROWSER_MEDIAPLAYER_MANAGER_H_
#define XWALK_TIZEN_BROWSER_BROWSER_MEDIAPLAYER_MANAGER_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "content/public/browser/web_contents_observer.h"
#include "url/gurl.h"
#include "xwalk/tizen/browser/murphy_resource_manager.h"
#include "xwalk/tizen/browser/murphy_resource.h"

namespace tizen {

// This class manages all MurphyResource objects in the browser process. It
// receives control operations from the render process, and forwards them to
// corresponding MurphyResource object. Callbacks from MurphyResource objects
// are converted to IPCs and then sent to the render process.
class CONTENT_EXPORT BrowserMediaPlayerManager
    : public content::WebContentsObserver {
 public:
  virtual ~BrowserMediaPlayerManager();

  // Returns a new instance.
  static BrowserMediaPlayerManager* Create(
      content::RenderViewHost* render_view_host);

  // WebContentsObserver overrides.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual MurphyResource* GetMurphyResource(
      MediaPlayerID player_id) OVERRIDE;

  void ResourceNotifyCallback(mrp_res_resource_state_t state,
      MediaPlayerID player_id);

 protected:
  explicit BrowserMediaPlayerManager(content::RenderViewHost* render_view_host);

 private:
  virtual void OnInitialize(MediaPlayerID player_id,
      int process_id, const GURL& url);
  virtual void OnDestroyAllMediaPlayers();
  virtual void OnDestroyPlayer(MediaPlayerID player_id);
  virtual void OnPause(MediaPlayerID player_id);
  virtual void OnStart(MediaPlayerID player_id);

  // Adds a murphy resource for the given player to the list.
  void AddMurphyResource(MurphyResource* resource);

  // Removes the murphy resource of given |player_id|.
  void RemoveMurphyResource(MediaPlayerID player_id);

  scoped_ptr<MurphyResourceManager> resource_manager_;
  ScopedVector<MurphyResource> murphy_resources_;

  DISALLOW_COPY_AND_ASSIGN(BrowserMediaPlayerManager);
};

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_BROWSER_MEDIAPLAYER_MANAGER_H_
