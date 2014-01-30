// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_url_request_context_getter.h"

#include <algorithm>
#include <vector>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "net/cert/cert_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/dns/mapped_host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/default_server_bound_cert_store.h"
#include "net/ssl/server_bound_cert_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/protocol_intercept_job_factory.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "xwalk/runtime/browser/runtime_network_delegate.h"

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/android/cookie_manager.h"
#include "xwalk/runtime/browser/android/net/android_protocol_handler.h"
#include "xwalk/runtime/browser/android/net/url_constants.h"
#include "xwalk/runtime/browser/android/net/xwalk_url_request_job_factory.h"
#include "xwalk/runtime/browser/android/xwalk_request_interceptor.h"
#endif

using content::BrowserThread;

namespace xwalk {

RuntimeURLRequestContextGetter::RuntimeURLRequestContextGetter(
    bool ignore_certificate_errors,
    const base::FilePath& base_path,
    base::MessageLoop* io_loop,
    base::MessageLoop* file_loop,
    content::ProtocolHandlerMap* protocol_handlers)
    : ignore_certificate_errors_(ignore_certificate_errors),
      base_path_(base_path),
      io_loop_(io_loop),
      file_loop_(file_loop) {
  // Must first be created on the UI thread.
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  std::swap(protocol_handlers_, *protocol_handlers);

  // We must create the proxy config service on the UI loop on Linux because it
  // must synchronously run on the glib message loop. This will be passed to
  // the URLRequestContextStorage on the IO thread in GetURLRequestContext().
  proxy_config_service_.reset(
      net::ProxyService::CreateSystemProxyConfigService(
          io_loop_->message_loop_proxy(), file_loop_));
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
    storage_->set_cookie_store(xwalk::GetCookieMonster());
#else
    storage_->set_cookie_store(new net::CookieMonster(NULL, NULL));
#endif
    storage_->set_server_bound_cert_service(new net::ServerBoundCertService(
        new net::DefaultServerBoundCertStore(NULL),
        base::WorkerPool::GetTaskRunner(true)));
    storage_->set_http_user_agent_settings(
        new net::StaticHttpUserAgentSettings("en-us,en", base::EmptyString()));

    scoped_ptr<net::HostResolver> host_resolver(
        net::HostResolver::CreateDefaultResolver(NULL));

    storage_->set_cert_verifier(net::CertVerifier::CreateDefault());
    storage_->set_transport_security_state(new net::TransportSecurityState);
    storage_->set_proxy_service(
        net::ProxyService::CreateUsingSystemProxyResolver(
        proxy_config_service_.release(),
        0,
        NULL));
    storage_->set_ssl_config_service(new net::SSLConfigServiceDefaults);
    storage_->set_http_auth_handler_factory(
        net::HttpAuthHandlerFactory::CreateDefault(host_resolver.get()));
    storage_->set_http_server_properties(scoped_ptr<net::HttpServerProperties>(
        new net::HttpServerPropertiesImpl));

    base::FilePath cache_path = base_path_.Append(FILE_PATH_LITERAL("Cache"));
    net::HttpCache::DefaultBackend* main_backend =
        new net::HttpCache::DefaultBackend(
            net::DISK_CACHE,
            net::CACHE_BACKEND_DEFAULT,
            cache_path,
            0,
            BrowserThread::GetMessageLoopProxyForThread(
                BrowserThread::CACHE));

    net::HttpNetworkSession::Params network_session_params;
    network_session_params.cert_verifier =
        url_request_context_->cert_verifier();
    network_session_params.transport_security_state =
        url_request_context_->transport_security_state();
    network_session_params.server_bound_cert_service =
        url_request_context_->server_bound_cert_service();
    network_session_params.proxy_service =
        url_request_context_->proxy_service();
    network_session_params.ssl_config_service =
        url_request_context_->ssl_config_service();
    network_session_params.http_auth_handler_factory =
        url_request_context_->http_auth_handler_factory();
    network_session_params.network_delegate =
        network_delegate_.get();
    network_session_params.http_server_properties =
        url_request_context_->http_server_properties();
    network_session_params.ignore_certificate_errors =
        ignore_certificate_errors_;

    // Give |storage_| ownership at the end in case it's |mapped_host_resolver|.
    storage_->set_host_resolver(host_resolver.Pass());
    network_session_params.host_resolver =
        url_request_context_->host_resolver();

    net::HttpCache* main_cache = new net::HttpCache(
        network_session_params, main_backend);
    storage_->set_http_transaction_factory(main_cache);

#if defined(OS_ANDROID)
    scoped_ptr<XWalkURLRequestJobFactory> job_factory_impl(
        new XWalkURLRequestJobFactory);
#else
    scoped_ptr<net::URLRequestJobFactoryImpl> job_factory_impl(
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
          it->first, it->second.release());
      DCHECK(set_protocol);
    }
    protocol_handlers_.clear();

    // Step 2:
    // Add new basic schemes.
    set_protocol = job_factory_impl->SetProtocolHandler(
        content::kDataScheme,
        new net::DataProtocolHandler);
    DCHECK(set_protocol);
    set_protocol = job_factory_impl->SetProtocolHandler(
        content::kFileScheme,
        new net::FileProtocolHandler(
            content::BrowserThread::GetBlockingPool()->
            GetTaskRunnerWithShutdownBehavior(
                base::SequencedWorkerPool::SKIP_ON_SHUTDOWN)));
    DCHECK(set_protocol);

    // Step 3:
    // Add the scheme interceptors.
    // Create a chain of URLRequestJobFactories. The handlers will be invoked
    // in the order in which they appear in the protocol_handlers vector.
    typedef std::vector<net::URLRequestJobFactory::ProtocolHandler*>
        ProtocolHandlerVector;
    ProtocolHandlerVector protocol_interceptors;

#if defined(OS_ANDROID)
    protocol_interceptors.push_back(
        CreateContentSchemeProtocolHandler().release());
    protocol_interceptors.push_back(
        CreateAssetFileProtocolHandler().release());
    protocol_interceptors.push_back(
        CreateAppSchemeProtocolHandler().release());
    // The XWalkRequestInterceptor must come after the content and asset
    // file job factories. This for WebViewClassic compatibility where it
    // was not possible to intercept resource loads to resolvable content://
    // and file:// URIs.
    // This logical dependency is also the reason why the Content
    // ProtocolHandler has to be added as a ProtocolInterceptJobFactory rather
    // than via SetProtocolHandler.
    protocol_interceptors.push_back(new XWalkRequestInterceptor());
#endif

    // The chain of responsibility will execute the handlers in reverse to the
    // order in which the elements of the chain are created.
    scoped_ptr<net::URLRequestJobFactory> job_factory(
        job_factory_impl.PassAs<net::URLRequestJobFactory>());
    for (ProtocolHandlerVector::reverse_iterator
             i = protocol_interceptors.rbegin();
         i != protocol_interceptors.rend();
         ++i) {
      job_factory.reset(new net::ProtocolInterceptJobFactory(
          job_factory.Pass(), make_scoped_ptr(*i)));
    }

    storage_->set_job_factory(job_factory.release());
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

}  // namespace xwalk
