// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_MEDIA_TIZEN_MEDIA_PLAYER_MANAGER_IMPL_H_
#define XWALK_RUNTIME_BROWSER_MEDIA_TIZEN_MEDIA_PLAYER_MANAGER_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "content/public/browser/render_view_host_observer.h"
#include "googleurl/src/gurl.h"
#include "xwalk/runtime/browser/media/tizen_audio_session_manager.h"

namespace content {

// This class manages all TizenAudioSessionManager objects in the browser
// process. It receives control operations from the render process, and
// forwards them to corresponding AudioSessionManager object. Callbacks
// from AudioSessionManager objects are converted to IPCs and then sent
// to the render process.
class CONTENT_EXPORT TizenMediaPlayerManagerImpl
    : public RenderViewHostObserver {
 public:
  TizenMediaPlayerManagerImpl(RenderViewHost* render_view_host);
  virtual ~TizenMediaPlayerManagerImpl();

  // RenderViewHostObserver overrides.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual TizenAudioSessionManager* GetAudioSessionManager(
      int player_id) OVERRIDE;

  ASM_cb_result_t AudioSessionEventPause(
      ASM_event_sources_t eventSource, int player_id);
  ASM_cb_result_t AudioSessionEventPlay(
      ASM_event_sources_t eventSource, int player_id);

 private:
  virtual void OnInitialize(int player_id, int process_id, const GURL& url);
  virtual void OnDestroyAllMediaPlayers();
  virtual void OnDestroyPlayer(int player_id);
  virtual void OnPause(int player_id);
  virtual void OnStart(int player_id);

  // Adds a audio session manager for the given player to the list.
  void AddAudioSessionManager(TizenAudioSessionManager* player);

  // Removes the audio session manager with the specified id.
  void RemoveAudioSessionManager(int player_id);

  ScopedVector<TizenAudioSessionManager> audio_session_managers_;

  DISALLOW_COPY_AND_ASSIGN(TizenMediaPlayerManagerImpl);
};

}  // namespace content

#endif  // XWALK_RUNTIME_BROWSER_MEDIA_TIZEN_MEDIA_PLAYER_MANAGER_IMPL_H_
