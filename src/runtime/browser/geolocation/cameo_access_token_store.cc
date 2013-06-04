// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/runtime/browser/geolocation/cameo_access_token_store.h"

#include "base/bind.h"
#include "base/message_loop.h"

CameoAccessTokenStore::CameoAccessTokenStore(
    net::URLRequestContextGetter* request_context)
    : request_context_(request_context) {
}

CameoAccessTokenStore::~CameoAccessTokenStore() {
}

void CameoAccessTokenStore::LoadAccessTokens(
    const LoadAccessTokensCallbackType& callback) {
  MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&CameoAccessTokenStore::DidLoadAccessTokens,
                 request_context_, callback));
}

void CameoAccessTokenStore::DidLoadAccessTokens(
    net::URLRequestContextGetter* request_context,
    const LoadAccessTokensCallbackType& callback) {
  AccessTokenSet access_token_set;
  callback.Run(access_token_set, request_context);
}

void CameoAccessTokenStore::SaveAccessToken(
    const GURL& server_url, const string16& access_token) {
}
