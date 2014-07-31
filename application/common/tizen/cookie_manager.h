// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_COOKIE_MANAGER_H_
#define XWALK_APPLICATION_COMMON_TIZEN_COOKIE_MANAGER_H_

#include <string>

#include "net/cookies/canonical_cookie.h"

#include "xwalk/runtime/browser/runtime_context.h"

namespace xwalk {

class CookieManager {
 public:
  CookieManager(const std::string& app_id, RuntimeContext* runtime_context);
  void RemoveAllCookies();
  void SetUserAgentString(content::RenderProcessHost* render_process_host,
                          const std::string& user_agent_string);
 private:
  void CookieDeleted(bool success);
  void DeleteSessionOnlyOriginCookies(const net::CookieList& cookies);
  void DeleteCookiesOnIOThread(const std::string& url,
                               const std::string& cookie_name);

  const std::string app_id_;
  RuntimeContext* runtime_context_;
};

}  // namespace xwalk
#endif  // XWALK_APPLICATION_COMMON_TIZEN_COOKIE_MANAGER_H_
