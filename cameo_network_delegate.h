#ifndef CAMEO_CAMEO_NETWORK_DELEGATE_H_
#define CAMEO_CAMEO_NETWORK_DELEGATE_H_

#include "net/base/net_errors.h"
#include "net/base/network_delegate.h"

class CameoNetworkDelegate : public net::NetworkDelegate {
 public:
  CameoNetworkDelegate();
  virtual ~CameoNetworkDelegate();

 private:
  // net::NetworkDelegate implementation.
  virtual int OnBeforeURLRequest(net::URLRequest* request,
                                 const net::CompletionCallback& callback,
                                 GURL* new_url) OVERRIDE {
    return net::OK;
  }

  virtual int OnBeforeSendHeaders(net::URLRequest* request,
                                  const net::CompletionCallback& callback,
                                  net::HttpRequestHeaders* headers) OVERRIDE {
    return net::OK;
  }

  virtual void OnSendHeaders(net::URLRequest* request,
                             const net::HttpRequestHeaders& headers) OVERRIDE {}
  virtual int OnHeadersReceived(
      net::URLRequest* request,
      const net::CompletionCallback& callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>*
      override_response_headers) OVERRIDE {
    return net::OK;
  }

  virtual void OnBeforeRedirect(net::URLRequest* request,
                                const GURL& new_location) OVERRIDE {}
  virtual void OnResponseStarted(net::URLRequest* request) OVERRIDE {}
  virtual void OnRawBytesRead(const net::URLRequest& request,
                              int bytes_read) OVERRIDE {}
  virtual void OnCompleted(net::URLRequest* request, bool started) OVERRIDE {}
  virtual void OnURLRequestDestroyed(net::URLRequest* request) OVERRIDE {}
  virtual void OnPACScriptError(int line_number,
                                const string16& error) OVERRIDE {}
  virtual AuthRequiredResponse OnAuthRequired(
      net::URLRequest* request,
      const net::AuthChallengeInfo& auth_info,
      const AuthCallback& callback,
      net::AuthCredentials* credentials) OVERRIDE {
    return AUTH_REQUIRED_RESPONSE_NO_ACTION;
  }

  virtual bool OnCanGetCookies(const net::URLRequest& request,
                               const net::CookieList& cookie_list) OVERRIDE;
  virtual bool OnCanSetCookie(const net::URLRequest& request,
                              const std::string& cookie_line,
                              net::CookieOptions* options) OVERRIDE;
  virtual bool OnCanAccessFile(const net::URLRequest& request,
                               const base::FilePath& path) const OVERRIDE {
    return true;
  }

  virtual bool OnCanThrottleRequest(
      const net::URLRequest& request) const OVERRIDE {
    return false;
  }

  virtual int OnBeforeSocketStreamConnect(
      net::SocketStream* stream,
      const net::CompletionCallback& callback) OVERRIDE {
    return net::OK;
  }

  virtual void OnRequestWaitStateChange(const net::URLRequest& request,
                                        RequestWaitState state) OVERRIDE {}

  DISALLOW_COPY_AND_ASSIGN(CameoNetworkDelegate);
};

#endif  // CAMEO_CAMEO_NETWORK_DELEGATE_H_
