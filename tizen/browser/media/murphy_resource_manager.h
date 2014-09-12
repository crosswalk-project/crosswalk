// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_TIZEN_BROWSER_MEDIA_MURPHY_RESOURCE_MANAGER_H_
#define XWALK_TIZEN_BROWSER_MEDIA_MURPHY_RESOURCE_MANAGER_H_

#include <murphy/plugins/resource-native/libmurphy-resource/resource-api.h>

#include "base/memory/scoped_ptr.h"
#include "xwalk/tizen/browser/media/murphy_mainloop.h"

namespace tizen {
class BrowserMediaPlayerManager;

class MurphyResourceManager {
 public:
  MurphyResourceManager();
  ~MurphyResourceManager();

  void ConnectToMurphy();
  void DisconnectFromMurphy();
  bool IsConnected() const;

  mrp_res_context_t* GetContext();

 private:
  mrp_res_context_t* murphy_context_;
  scoped_ptr<MurphyMainloop> murphy_mainloop_;
  mrp_mainloop_t* mainloop_;
};

}  // namespace tizen

#endif  // XWALK_TIZEN_BROWSER_MEDIA_MURPHY_RESOURCE_MANAGER_H_
