// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_MURPHY_RESOURCE_MANAGER_H_
#define XWALK_TIZEN_BROWSER_MURPHY_RESOURCE_MANAGER_H_

#include <murphy/plugins/resource-native/libmurphy-resource/resource-api.h>

#include "base/memory/scoped_ptr.h"
#include "xwalk/tizen/browser/murphy_mainloop.h"

namespace tizen {
class BrowserMediaPlayerManager;

class MurphyResourceManager {
 public:
  explicit MurphyResourceManager(BrowserMediaPlayerManager* manager);
  virtual ~MurphyResourceManager();

  void ConnectToMurphy();
  void DisconnectFromMurphy();
  bool IsConnected() const;

  mrp_res_context_t* GetContext();

  BrowserMediaPlayerManager* media_player_manager() {
    return manager_;
  }

 private:
  BrowserMediaPlayerManager* manager_;
  mrp_res_context_t* murphy_context_;
  scoped_ptr<MurphyMainloop> murphy_mainloop_;
  mrp_mainloop_t* mainloop_;
};

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_MURPHY_RESOURCE_MANAGER_H_
