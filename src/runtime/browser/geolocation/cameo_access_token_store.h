// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_RUNTIME_BROWSER_GEOLOCATION_CAMEO_ACCESS_TOKEN_STORE_H_
#define CAMEO_SRC_RUNTIME_BROWSER_GEOLOCATION_CAMEO_ACCESS_TOKEN_STORE_H_

#include "content/public/browser/access_token_store.h"

class CameoAccessTokenStore : public content::AccessTokenStore {
 public:
  explicit CameoAccessTokenStore(net::URLRequestContextGetter* request_context);

 private:
  virtual ~CameoAccessTokenStore();

  // AccessTokenStore
  virtual void LoadAccessTokens(
      const LoadAccessTokensCallbackType& callback) OVERRIDE;

  virtual void SaveAccessToken(
      const GURL& server_url, const string16& access_token) OVERRIDE;

  static void DidLoadAccessTokens(
      net::URLRequestContextGetter* request_context,
      const LoadAccessTokensCallbackType& callback);

  net::URLRequestContextGetter* request_context_;

  DISALLOW_COPY_AND_ASSIGN(CameoAccessTokenStore);
};

#endif  // CAMEO_SRC_RUNTIME_BROWSER_GEOLOCATION_CAMEO_ACCESS_TOKEN_STORE_H_
