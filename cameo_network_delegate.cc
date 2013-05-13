#include "cameo/cameo_network_delegate.h"

#include "net/base/static_cookie_policy.h"
#include "net/url_request/url_request.h"

CameoNetworkDelegate::CameoNetworkDelegate() {
}

CameoNetworkDelegate::~CameoNetworkDelegate() {
}

bool CameoNetworkDelegate::OnCanGetCookies(
    const net::URLRequest& request,
    const net::CookieList& cookie_list) {
  net::StaticCookiePolicy policy(net::StaticCookiePolicy::ALLOW_ALL_COOKIES);
  int rv = policy.CanGetCookies(request.url(),
                                request.first_party_for_cookies());
  return rv == net::OK;
}

bool CameoNetworkDelegate::OnCanSetCookie(
    const net::URLRequest& request,
    const std::string& cookie_line,
    net::CookieOptions* options) {
  net::StaticCookiePolicy policy(net::StaticCookiePolicy::ALLOW_ALL_COOKIES);
  int rv = policy.CanSetCookie(request.url(),
                               request.first_party_for_cookies());
  return rv == net::OK;
}
