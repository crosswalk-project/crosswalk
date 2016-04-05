// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/geolocation/xwalk_access_token_store.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"

XWalkAccessTokenStore::XWalkAccessTokenStore(
    net::URLRequestContextGetter* request_context)
    : request_context_(request_context) {
}

XWalkAccessTokenStore::~XWalkAccessTokenStore() {
}

void XWalkAccessTokenStore::LoadAccessTokens(
    const LoadAccessTokensCallback& callback) {
  base::MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&XWalkAccessTokenStore::DidLoadAccessTokens,
                 request_context_, callback));
}

void XWalkAccessTokenStore::DidLoadAccessTokens(
    net::URLRequestContextGetter* request_context,
    const LoadAccessTokensCallback& callback) {
  AccessTokenMap access_token_map;
  callback.Run(access_token_map, request_context);
}

void XWalkAccessTokenStore::SaveAccessToken(
    const GURL& server_url, const base::string16& access_token) {
}
