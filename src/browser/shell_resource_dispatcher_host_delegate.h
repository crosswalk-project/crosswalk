// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define CAMEO_SRC_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace cameo {

class ShellResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  ShellResourceDispatcherHostDelegate();
  virtual ~ShellResourceDispatcherHostDelegate();

  // ResourceDispatcherHostDelegate implementation.
  virtual bool AcceptAuthRequest(net::URLRequest* request,
                                 net::AuthChallengeInfo* auth_info) OVERRIDE;
  virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info, net::URLRequest* request) OVERRIDE;

 private:
  base::Callback<void()> login_request_callback_;

  DISALLOW_COPY_AND_ASSIGN(ShellResourceDispatcherHostDelegate);
};

}  // namespace content

#endif  // CAMEO_SRC_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
