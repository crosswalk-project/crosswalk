// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_dev_tools_server.h"

#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "base/android/jni_string.h"
#include "base/basictypes.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "blink_upstream_version.h"  // NOLINT
#include "content/public/browser/android/devtools_auth.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_manager_delegate.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "grit/xwalk_resources.h"
#include "jni/XWalkDevToolsServer_jni.h"
#include "net/socket/unix_domain_listen_socket_posix.h"
#include "ui/base/resource/resource_bundle.h"

using content::DevToolsAgentHost;
using content::RenderViewHost;
using content::WebContents;

namespace {

// FIXME(girish): The frontend URL needs to be served from the domain below
// for remote debugging to work in chrome (see chrome's devtools_ui.cc).
// Currently, the chrome version is hardcoded because of this dependancy.
const char kFrontEndURL[] =
    "http://chrome-devtools-frontend.appspot.com/serve_rev/%s/devtools.html";

bool AuthorizeSocketAccessWithDebugPermission(
     const net::UnixDomainServerSocket::Credentials& credentials) {
  JNIEnv* env = base::android::AttachCurrentThread();
  return xwalk::Java_XWalkDevToolsServer_checkDebugPermission(
      env, base::android::GetApplicationContext(),
      credentials.process_id, credentials.user_id) ||
      content::CanUserConnectToDevTools(credentials);
}

// Delegate implementation for the devtools http handler on android. A new
// instance of this gets created each time devtools is enabled.
class XWalkAndroidDevToolsHttpHandlerDelegate
  : public content::DevToolsHttpHandlerDelegate {
 public:
  XWalkAndroidDevToolsHttpHandlerDelegate() {
  }

  virtual std::string GetDiscoveryPageHTML() OVERRIDE {
    return ResourceBundle::GetSharedInstance().GetRawDataResource(
        IDR_DEVTOOLS_FRONTEND_PAGE_HTML).as_string();
  }

  virtual bool BundlesFrontendResources() OVERRIDE {
    return false;
  }

  virtual base::FilePath GetDebugFrontendDir() OVERRIDE {
    return base::FilePath();
  }

  virtual scoped_ptr<net::StreamListenSocket> CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) OVERRIDE {
    return scoped_ptr<net::StreamListenSocket>();
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkAndroidDevToolsHttpHandlerDelegate);
};

// Factory for UnixDomainServerSocket.
class UnixDomainServerSocketFactory
    : public content::DevToolsHttpHandler::ServerSocketFactory {
 public:
  explicit UnixDomainServerSocketFactory(
      const std::string& socket_name,
      const net::UnixDomainServerSocket::AuthCallback& auth_callback)
      : content::DevToolsHttpHandler::ServerSocketFactory(socket_name, 0, 1),
      auth_callback_(auth_callback) {}

 private:
  // content::DevToolsHttpHandler::ServerSocketFactory.
  virtual scoped_ptr<net::ServerSocket> Create() const OVERRIDE {
    return scoped_ptr<net::ServerSocket>(
        new net::UnixDomainServerSocket(
            auth_callback_,
            true /* use_abstract_namespace */));
  }

  const net::UnixDomainServerSocket::AuthCallback auth_callback_;
  DISALLOW_COPY_AND_ASSIGN(UnixDomainServerSocketFactory);
};

}  // namespace

namespace xwalk {

XWalkDevToolsServer::XWalkDevToolsServer(const std::string& socket_name)
    : socket_name_(socket_name),
      protocol_handler_(NULL),
      allow_debug_permission_(false),
      allow_socket_access_(false) {
}

XWalkDevToolsServer::~XWalkDevToolsServer() {
  Stop();
}

// Allow connection from uid specified using AllowConnectionFromUid to devtools
// server. This supports the XDK usage: debug bridge wrapper runs in a separate
// process and connects to the devtools server.
bool XWalkDevToolsServer::CanUserConnectToDevTools(
    const net::UnixDomainServerSocket::Credentials& credentials) {
  if (allow_socket_access_)
    return true;
  if (allow_debug_permission_)
    return AuthorizeSocketAccessWithDebugPermission(credentials);
  return content::CanUserConnectToDevTools(credentials);
}

void XWalkDevToolsServer::Start(bool allow_debug_permission,
                                bool allow_socket_access) {
  allow_debug_permission_ = allow_debug_permission;
  allow_socket_access_ = allow_socket_access;
  if (protocol_handler_)
    return;

  net::UnixDomainServerSocket::AuthCallback auth_callback =
      base::Bind(&XWalkDevToolsServer::CanUserConnectToDevTools,
                 base::Unretained(this));

  scoped_ptr<content::DevToolsHttpHandler::ServerSocketFactory> factory(
      new UnixDomainServerSocketFactory(socket_name_, auth_callback));
  protocol_handler_ = content::DevToolsHttpHandler::Start(
      factory.Pass(),
      base::StringPrintf(kFrontEndURL, BLINK_UPSTREAM_REVISION),
      new XWalkAndroidDevToolsHttpHandlerDelegate(), base::FilePath());
}

void XWalkDevToolsServer::Stop() {
  if (!protocol_handler_)
    return;
  // Note that the call to Stop() below takes care of |protocol_handler_|
  // deletion.
  protocol_handler_->Stop();
  protocol_handler_ = NULL;
  allow_socket_access_ = false;
  allow_debug_permission_ = false;
}

bool XWalkDevToolsServer::IsStarted() const {
  return protocol_handler_;
}

bool RegisterXWalkDevToolsServer(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

static jlong InitRemoteDebugging(JNIEnv* env,
                                jobject obj,
                                jstring socketName) {
  XWalkDevToolsServer* server = new XWalkDevToolsServer(
      base::android::ConvertJavaStringToUTF8(env, socketName));
  return reinterpret_cast<intptr_t>(server);
}

static void DestroyRemoteDebugging(JNIEnv* env, jobject obj, jlong server) {
  delete reinterpret_cast<XWalkDevToolsServer*>(server);
}

static jboolean IsRemoteDebuggingEnabled(JNIEnv* env,
                                         jobject obj,
                                         jlong server) {
  return reinterpret_cast<XWalkDevToolsServer*>(server)->IsStarted();
}

static void SetRemoteDebuggingEnabled(JNIEnv* env,
                                      jobject obj,
                                      jlong server,
                                      jboolean enabled,
                                      jboolean allow_debug_permission,
                                      jboolean allow_socket_access) {
  XWalkDevToolsServer* devtools_server =
      reinterpret_cast<XWalkDevToolsServer*>(server);
  if (enabled == JNI_TRUE) {
    devtools_server->Start(allow_debug_permission == JNI_TRUE,
                           allow_socket_access == JNI_TRUE);
  } else {
    devtools_server->Stop();
  }
}

}  // namespace xwalk
