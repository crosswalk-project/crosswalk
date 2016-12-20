// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "net/cert/cert_verifier.h"
#include "net/cert/ct_policy_enforcer.h"
#include "net/cert/ct_policy_status.h"
#include "net/cert/ct_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/dns/mapped_host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/channel_id_service.h"
#include "net/ssl/default_channel_id_store.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_intercepting_job_factory.h"
#include "net/url_request/url_request_interceptor.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/browser/runtime_network_delegate.h"
#include "xwalk/runtime/common/xwalk_content_client.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_ANDROID)
#include "net/proxy/proxy_config_service_android.h"
#include "xwalk/runtime/browser/android/cookie_manager.h"
#include "xwalk/runtime/browser/android/net/android_protocol_handler.h"
#include "xwalk/runtime/browser/android/net/url_constants.h"
#include "xwalk/runtime/browser/android/net/xwalk_cookie_store_wrapper.h"
#include "xwalk/runtime/browser/android/net/xwalk_url_request_job_factory.h"
#include "xwalk/runtime/browser/android/xwalk_request_interceptor.h"
#endif

using content::BrowserThread;

namespace xwalk {

namespace {

// TODO(rakuco): should Crosswalk's release cycle ever align with Chromium's,
// we should use Chromium's Certificate Transparency policy and stop ignoring
// CT information with the classes below.
// See the discussion in http://crbug.com/669978 for more information.

// A CTVerifier which ignores Certificate Transparency information.
class IgnoresCTVerifier : public net::CTVerifier {
 public:
  IgnoresCTVerifier() = default;
  ~IgnoresCTVerifier() override = default;

  int Verify(net::X509Certificate* cert,
             const std::string& stapled_ocsp_response,
             const std::string& sct_list_from_tls_extension,
             net::ct::CTVerifyResult* result,
             const net::BoundNetLog& net_log) override {
    return net::OK;
  }

  void SetObserver(Observer* observer) override {}
};

// A CTPolicyEnforcer that accepts all certificates.
class IgnoresCTPolicyEnforcer : public net::CTPolicyEnforcer {
 public:
  IgnoresCTPolicyEnforcer() = default;
  ~IgnoresCTPolicyEnforcer() override = default;

  net::ct::CertPolicyCompliance DoesConformToCertPolicy(
      net::X509Certificate* cert,
      const net::SCTList& verified_scts,
      const net::BoundNetLog& net_log) override {
    return net::ct::CertPolicyCompliance::CERT_POLICY_COMPLIES_VIA_SCTS;
  }

  net::ct::EVPolicyCompliance DoesConformToCTEVPolicy(
      net::X509Certificate* cert,
      const net::ct::EVCertsWhitelist* ev_whitelist,
      const net::SCTList& verified_scts,
      const net::BoundNetLog& net_log) override {
    return net::ct::EVPolicyCompliance::EV_POLICY_DOES_NOT_APPLY;
  }
};

}  // namespace

int GetDiskCacheSize() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  if (!command_line->HasSwitch(switches::kDiskCacheSize))
      return 0;

  std::string str_value = command_line->GetSwitchValueASCII(
      switches::kDiskCacheSize);

  int size = 0;
  if (!base::StringToInt(str_value, &size)) {
      LOG(ERROR) << "The value " << str_value
                 << " can not be converted to integer, ignoring!";
  }

  return size;
}

RuntimeURLRequestContextGetter::RuntimeURLRequestContextGetter(
    bool ignore_certificate_errors,
    const base::FilePath& base_path,
    base::MessageLoop* io_loop,
    base::MessageLoop* file_loop,
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors)
    : ignore_certificate_errors_(ignore_certificate_errors),
      base_path_(base_path),
      io_loop_(io_loop),
      file_loop_(file_loop),
      request_interceptors_(std::move(request_interceptors)) {
  // Must first be created on the UI thread.
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  std::swap(protocol_handlers_, *protocol_handlers);

  // We must create the proxy config service on the UI loop on Linux because it
  // must synchronously run on the glib message loop. This will be passed to
  // the URLRequestContextStorage on the IO thread in GetURLRequestContext().
#if defined(OS_ANDROID)
  proxy_config_service_ = net::ProxyService::CreateSystemProxyConfigService(
      io_loop_->task_runner(), file_loop_->task_runner());
  net::ProxyConfigServiceAndroid* android_config_service =
      static_cast<net::ProxyConfigServiceAndroid*>(proxy_config_service_.get());
  android_config_service->set_exclude_pac_url(true);
#else
  proxy_config_service_ = net::ProxyService::CreateSystemProxyConfigService(
      io_loop_->task_runner(), file_loop_->task_runner());
#endif
}

RuntimeURLRequestContextGetter::~RuntimeURLRequestContextGetter() {
}

net::URLRequestContext* RuntimeURLRequestContextGetter::GetURLRequestContext() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

  if (!url_request_context_) {
    url_request_context_.reset(new net::URLRequestContext());
    network_delegate_.reset(new RuntimeNetworkDelegate);
    url_request_context_->set_network_delegate(network_delegate_.get());
    storage_.reset(
        new net::URLRequestContextStorage(url_request_context_.get()));
#if defined(OS_ANDROID)
    storage_->set_cookie_store(base::WrapUnique(new XWalkCookieStoreWrapper));
#else
    content::CookieStoreConfig cookie_config(base_path_.Append(
        application::kCookieDatabaseFilename),
        content::CookieStoreConfig::PERSISTANT_SESSION_COOKIES,
        NULL, NULL);

    cookie_config.cookieable_schemes.insert(
        cookie_config.cookieable_schemes.begin(),
        net::CookieMonster::kDefaultCookieableSchemes,
        net::CookieMonster::kDefaultCookieableSchemes +
        net::CookieMonster::kDefaultCookieableSchemesCount);
    cookie_config.cookieable_schemes.push_back(application::kApplicationScheme);
    cookie_config.cookieable_schemes.push_back(content::kChromeDevToolsScheme);

    auto cookie_store = content::CreateCookieStore(cookie_config);
    storage_->set_cookie_store(std::move(cookie_store));
#endif
    storage_->set_channel_id_service(base::WrapUnique(new net::ChannelIDService(
        new net::DefaultChannelIDStore(NULL),
        base::WorkerPool::GetTaskRunner(true))));
    storage_->set_http_user_agent_settings(base::WrapUnique(
        new net::StaticHttpUserAgentSettings("en-us,en",
                                             xwalk::GetUserAgent())));

    std::unique_ptr<net::HostResolver> host_resolver(
        net::HostResolver::CreateDefaultResolver(NULL));

    storage_->set_cert_verifier(net::CertVerifier::CreateDefault());
    storage_->set_transport_security_state(
        base::WrapUnique(new net::TransportSecurityState));

    // We consciously ignore certificate transparency checks at the moment
    // because we risk ignoring valid logs or accepting unqualified logs since
    // Crosswalk's release schedule does not match Chromium's. Additionally,
    // all the CT verification mechanisms stop working 70 days after a build is
    // made, so we would also need to release more quickly and users would need
    // to update their apps as well.
    // See the discussion in http://crbug.com/669978 for more information.
    storage_->set_cert_transparency_verifier(
        base::WrapUnique(new IgnoresCTVerifier));
    storage_->set_ct_policy_enforcer(
        base::WrapUnique(new IgnoresCTPolicyEnforcer));

#if defined(OS_ANDROID)
    // Android provides a local HTTP proxy that handles all the proxying.
    // Create the proxy without a resolver since we rely
    // on this local HTTP proxy.
    storage_->set_proxy_service(
        net::ProxyService::CreateWithoutProxyResolver(
        std::move(proxy_config_service_),
        NULL));
#else
    storage_->set_proxy_service(
        net::ProxyService::CreateUsingSystemProxyResolver(
        std::move(proxy_config_service_),
        0,
        NULL));
#endif
    storage_->set_ssl_config_service(new net::SSLConfigServiceDefaults);
    storage_->set_http_auth_handler_factory(
        net::HttpAuthHandlerFactory::CreateDefault(host_resolver.get()));
    storage_->set_http_server_properties(std::unique_ptr<net::HttpServerProperties>(
        new net::HttpServerPropertiesImpl));

    base::FilePath cache_path = base_path_.Append(FILE_PATH_LITERAL("Cache"));
    std::unique_ptr<net::HttpCache::DefaultBackend> main_backend(
        new net::HttpCache::DefaultBackend(
            net::DISK_CACHE,
            net::CACHE_BACKEND_DEFAULT,
            cache_path,
            GetDiskCacheSize(),
            BrowserThread::GetMessageLoopProxyForThread(
                BrowserThread::CACHE)));

    net::HttpNetworkSession::Params network_session_params;
    network_session_params.cert_verifier =
        url_request_context_->cert_verifier();
    network_session_params.transport_security_state =
        url_request_context_->transport_security_state();
    network_session_params.cert_transparency_verifier =
        url_request_context_->cert_transparency_verifier();
    network_session_params.ct_policy_enforcer =
        url_request_context_->ct_policy_enforcer();
    network_session_params.channel_id_service =
        url_request_context_->channel_id_service();
    network_session_params.proxy_service =
        url_request_context_->proxy_service();
    network_session_params.ssl_config_service =
        url_request_context_->ssl_config_service();
    network_session_params.http_auth_handler_factory =
        url_request_context_->http_auth_handler_factory();
    network_session_params.http_server_properties =
        url_request_context_->http_server_properties();
    network_session_params.ignore_certificate_errors =
        ignore_certificate_errors_;

    // Give |storage_| ownership at the end in case it's |mapped_host_resolver|.
    storage_->set_host_resolver(std::move(host_resolver));
    network_session_params.host_resolver =
        url_request_context_->host_resolver();

    storage_->set_http_network_session(
        base::WrapUnique(new net::HttpNetworkSession(network_session_params)));
    storage_->set_http_transaction_factory(
        base::WrapUnique(new net::HttpCache(storage_->http_network_session(),
                        std::move(main_backend),
                        false /* set_up_quic_server_info */)));
#if defined(OS_ANDROID)
    std::unique_ptr<XWalkURLRequestJobFactory> job_factory_impl(
        new XWalkURLRequestJobFactory);
#else
    std::unique_ptr<net::URLRequestJobFactoryImpl> job_factory_impl(
        new net::URLRequestJobFactoryImpl);
#endif

    bool set_protocol;

    // Step 1:
    // Install all the default schemes for crosswalk.
    for (content::ProtocolHandlerMap::iterator it =
             protocol_handlers_.begin();
         it != protocol_handlers_.end();
         ++it) {
      set_protocol = job_factory_impl->SetProtocolHandler(
          it->first, base::WrapUnique(it->second.release()));
      DCHECK(set_protocol);
    }
    protocol_handlers_.clear();

    // Step 2:
    // Add new basic schemes.
    set_protocol = job_factory_impl->SetProtocolHandler(
        url::kDataScheme,
        base::WrapUnique(new net::DataProtocolHandler));
    DCHECK(set_protocol);
    set_protocol = job_factory_impl->SetProtocolHandler(
        url::kFileScheme,
        base::WrapUnique(new net::FileProtocolHandler(
            content::BrowserThread::GetBlockingPool()->
            GetTaskRunnerWithShutdownBehavior(
                base::SequencedWorkerPool::SKIP_ON_SHUTDOWN))));
    DCHECK(set_protocol);

    // Step 3:
    // Add the scheme interceptors.
  // in the order in which they appear in the |request_interceptors| vector.
  typedef std::vector<net::URLRequestInterceptor*>
      URLRequestInterceptorVector;
  URLRequestInterceptorVector request_interceptors;

#if defined(OS_ANDROID)
    request_interceptors.push_back(
        CreateContentSchemeRequestInterceptor().release());
    request_interceptors.push_back(
        CreateAssetFileRequestInterceptor().release());
    request_interceptors.push_back(
        CreateAppSchemeRequestInterceptor().release());
    // The XWalkRequestInterceptor must come after the content and asset
    // file job factories. This for WebViewClassic compatibility where it
    // was not possible to intercept resource loads to resolvable content://
    // and file:// URIs.
    // This logical dependency is also the reason why the Content
    // ProtocolHandler has to be added as a ProtocolInterceptJobFactory rather
    // than via SetProtocolHandler.
    request_interceptors.push_back(new XWalkRequestInterceptor());
#endif

    // The chain of responsibility will execute the handlers in reverse to the
    // order in which the elements of the chain are created.
    std::unique_ptr<net::URLRequestJobFactory> job_factory(
        std::move(job_factory_impl));
    for (URLRequestInterceptorVector::reverse_iterator
             i = request_interceptors.rbegin();
         i != request_interceptors.rend();
         ++i) {
      job_factory.reset(new net::URLRequestInterceptingJobFactory(
          std::move(job_factory), base::WrapUnique(*i)));
    }

    // Set up interceptors in the reverse order.
    std::unique_ptr<net::URLRequestJobFactory> top_job_factory =
        std::move(job_factory);
    for (content::URLRequestInterceptorScopedVector::reverse_iterator i =
             request_interceptors_.rbegin();
         i != request_interceptors_.rend();
         ++i) {
      top_job_factory.reset(new net::URLRequestInterceptingJobFactory(
          std::move(top_job_factory), base::WrapUnique(*i)));
    }
    request_interceptors_.weak_clear();

    storage_->set_job_factory(std::move(top_job_factory));
  }

  return url_request_context_.get();
}

scoped_refptr<base::SingleThreadTaskRunner>
    RuntimeURLRequestContextGetter::GetNetworkTaskRunner() const {
  return BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO);
}

net::HostResolver* RuntimeURLRequestContextGetter::host_resolver() {
  return url_request_context_->host_resolver();
}

void RuntimeURLRequestContextGetter::UpdateAcceptLanguages(
    const std::string& accept_languages) {
  if (!storage_)
    return;
  storage_->set_http_user_agent_settings(base::WrapUnique(
      new net::StaticHttpUserAgentSettings(accept_languages,
                                           xwalk::GetUserAgent())));
}

}  // namespace xwalk
