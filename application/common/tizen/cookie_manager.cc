// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/tizen/cookie_manager.h"

#include "base/bind.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "net/cookies/cookie_monster.h"
#include "net/cookies/cookie_store.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"
#include "xwalk/runtime/common/xwalk_common_messages.h"

namespace xwalk {

CookieManager::CookieManager(
    const std::string& app_id,
    XWalkBrowserContext* browser_context)
      : app_id_(app_id),
        browser_context_(browser_context) {
}

void CookieManager::CookieDeleted(bool success) {
  if (!success)
    LOG(ERROR) << "Removal of cookie failed.";
}

void CookieManager::DeleteSessionOnlyOriginCookies(
    const net::CookieList& cookies) {
  net::URLRequestContext* request_context = browser_context_->
      GetURLRequestContextGetterById(
          std::string(app_id_))->GetURLRequestContext();
  if (!request_context)
    return;
  net::CookieMonster* cookie_monster =
      request_context->cookie_store()->GetCookieMonster();
  for (net::CookieList::const_iterator it = cookies.begin();
       it != cookies.end(); ++it) {
    cookie_monster->DeleteCanonicalCookieAsync(*it,
        base::Bind(&CookieManager::CookieDeleted,
                   base::Unretained(this)));
  }
}

void CookieManager::DeleteCookiesOnIOThread(
    const std::string& url,
    const std::string& cookie_name) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  net::URLRequestContext* request_context = browser_context_->
      GetURLRequestContextGetterById(app_id_)->GetURLRequestContext();
  net::CookieStore* cookie_store = request_context->cookie_store();
  cookie_store->GetCookieMonster()->GetAllCookiesAsync(
      base::Bind(&CookieManager::DeleteSessionOnlyOriginCookies,
                 base::Unretained(this)));
}

void CookieManager::RemoveAllCookies() {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE, base::Bind(
          &CookieManager::DeleteCookiesOnIOThread,
          base::Unretained(this), "", ""));
}

void CookieManager::SetUserAgentString(
    content::RenderProcessHost* render_process_host,
    const std::string& user_agent_string) {
  render_process_host->Send(
      new ViewMsg_UserAgentStringChanged(user_agent_string));
}

}  // namespace xwalk
