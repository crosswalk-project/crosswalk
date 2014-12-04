// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_GEOLOCATION_XWALK_ACCESS_TOKEN_STORE_H_
#define XWALK_RUNTIME_BROWSER_GEOLOCATION_XWALK_ACCESS_TOKEN_STORE_H_

#include "content/public/browser/access_token_store.h"

class XWalkAccessTokenStore : public content::AccessTokenStore {
 public:
  explicit XWalkAccessTokenStore(net::URLRequestContextGetter* request_context);

 private:
  virtual ~XWalkAccessTokenStore();

  // AccessTokenStore
  void LoadAccessTokens(
      const LoadAccessTokensCallbackType& callback) override;

  void SaveAccessToken(
      const GURL& server_url, const base::string16& access_token) override;

  static void DidLoadAccessTokens(
      net::URLRequestContextGetter* request_context,
      const LoadAccessTokensCallbackType& callback);

  net::URLRequestContextGetter* request_context_;

  DISALLOW_COPY_AND_ASSIGN(XWalkAccessTokenStore);
};

#endif  // XWALK_RUNTIME_BROWSER_GEOLOCATION_XWALK_ACCESS_TOKEN_STORE_H_
