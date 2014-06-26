// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_MURPHY_RESOURCE_H_
#define XWALK_TIZEN_BROWSER_MURPHY_RESOURCE_H_

#include "xwalk/tizen/browser/murphy_resource_manager.h"

namespace tizen {

typedef int MediaPlayerID;

class MurphyResource {
 public:
  MurphyResource(BrowserMediaPlayerManager* manager,
      MediaPlayerID player_id,
      MurphyResourceManager* resource_manager);
  ~MurphyResource();

  void AcquireResource();
  void ReleaseResource();

  mrp_res_resource_state_t GetResourceState() const { return resource_state_; }
  void SetResourceState(mrp_res_resource_state_t state) {
    resource_state_ = state;
  }

  MediaPlayerID player_id() const { return player_id_; }
  BrowserMediaPlayerManager* media_player_manager() {
    return manager_;
  }

 private:
  BrowserMediaPlayerManager* manager_;
  MediaPlayerID player_id_;
  MurphyResourceManager* resource_manager_;

  mrp_res_resource_set_t* resource_set_;
  mrp_res_resource_state_t resource_state_;
};

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_MURPHY_RESOURCE_H_
