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
#include "content/public/common/user_agent.h"
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
const char kTargetTypePage[] = "page";
const char kTargetTypeServiceWorker[] = "service_worker";
const char kTargetTypeOther[] = "other";

bool AuthorizeSocketAccessWithDebugPermission(
     const net::UnixDomainServerSocket::Credentials& credentials) {
  JNIEnv* env = base::android::AttachCurrentThread();
  return xwalk::Java_XWalkDevToolsServer_checkDebugPermission(
      env, base::android::GetApplicationContext(),
      credentials.process_id, credentials.user_id) ||
      content::CanUserConnectToDevTools(credentials);
}

class Target : public content::DevToolsTarget {
 public:
  explicit Target(scoped_refptr<content::DevToolsAgentHost> agent_host);

  virtual std::string GetId() const OVERRIDE { return agent_host_->GetId(); }
  virtual std::string GetType() const OVERRIDE {
      switch (agent_host_->GetType()) {
        case content::DevToolsAgentHost::TYPE_WEB_CONTENTS:
           return kTargetTypePage;
         case content::DevToolsAgentHost::TYPE_SERVICE_WORKER:
           return kTargetTypeServiceWorker;
         default:
           break;
       }
       return kTargetTypeOther;
     }
  virtual std::string GetTitle() const OVERRIDE {
    return agent_host_->GetTitle();
  }

  // TODO(hmin): Get the description about web contents view.
  virtual std::string GetDescription() const OVERRIDE { return std::string(); }
  virtual GURL GetURL() const OVERRIDE { return url_; }
  virtual GURL GetFaviconURL() const OVERRIDE { return GURL(); }
  virtual base::TimeTicks GetLastActivityTime() const OVERRIDE {
    return last_activity_time_;
  }
  virtual std::string GetParentId() const OVERRIDE { return std::string(); }
  virtual bool IsAttached() const OVERRIDE {
    return agent_host_->IsAttached();
  }
  virtual scoped_refptr<DevToolsAgentHost> GetAgentHost() const OVERRIDE {
    return agent_host_;
  }

  virtual bool Activate() const OVERRIDE {
    WebContents* web_contents = agent_host_->GetWebContents();
    if (!web_contents)
      return false;
    web_contents->GetDelegate()->ActivateContents(web_contents);
    return true;
  }

  virtual bool Close() const OVERRIDE { return false; }

 private:
  scoped_refptr<DevToolsAgentHost> agent_host_;
  std::string id_;
  std::string title_;
  GURL url_;
  GURL favicon_url_;
  base::TimeTicks last_activity_time_;
};

Target::Target(scoped_refptr<content::DevToolsAgentHost> agent_host)
    : agent_host_(agent_host) {
  if (content::WebContents* web_contents = agent_host_->GetWebContents()) {
    content::NavigationController& controller = web_contents->GetController();
    content::NavigationEntry* entry = controller.GetActiveEntry();
    if (entry != NULL && entry->GetURL().is_valid())
      favicon_url_ = entry->GetFavicon().url;
    last_activity_time_ = web_contents->GetLastActiveTime();
  }
}

// Delegate implementation for the devtools http handler on android. A new
// instance of this gets created each time devtools is enabled.
class XWalkDevToolsHttpHandlerDelegate
  : public content::DevToolsHttpHandlerDelegate {
 public:
  explicit XWalkDevToolsHttpHandlerDelegate(
    const net::UnixDomainServerSocket::AuthCallback& auth_callback)
    : auth_callback_(auth_callback) {
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
  const net::UnixDomainServerSocket::AuthCallback auth_callback_;
  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsHttpHandlerDelegate);
};

class XWalkDevToolsDelegate
  : public content::DevToolsManagerDelegate {
 public:
  virtual std::string GetPageThumbnailData(const GURL& url) OVERRIDE {
    return std::string();
  }

  virtual scoped_ptr<content::DevToolsTarget> CreateNewTarget(
      const GURL&) OVERRIDE {
    return scoped_ptr<content::DevToolsTarget>();
  }
  virtual void EnumerateTargets(TargetCallback callback) OVERRIDE {
    TargetList targets;
    content::DevToolsAgentHost::List agents =
        content::DevToolsAgentHost::GetOrCreateAll();
    for (content::DevToolsAgentHost::List::iterator it = agents.begin();
         it != agents.end(); ++it) {
      targets.push_back(new Target(*it));
    }
    callback.Run(targets);
  }
};

// Factory for UnixDomainServerSocket.
class UnixDomainServerSocketFactory
    : public content::DevToolsHttpHandler::ServerSocketFactory {
 public:
  explicit UnixDomainServerSocketFactory(const std::string& socket_name)
      : content::DevToolsHttpHandler::ServerSocketFactory(socket_name, 0, 1) {}

 private:
  // content::DevToolsHttpHandler::ServerSocketFactory.
  virtual scoped_ptr<net::ServerSocket> Create() const OVERRIDE {
    return scoped_ptr<net::ServerSocket>(
        new net::UnixDomainServerSocket(
            base::Bind(&content::CanUserConnectToDevTools),
            true /* use_abstract_namespace */));
  }

  DISALLOW_COPY_AND_ASSIGN(UnixDomainServerSocketFactory);
};

}  // namespace

namespace xwalk {

XWalkDevToolsServer::XWalkDevToolsServer(const std::string& socket_name)
    : socket_name_(socket_name),
      protocol_handler_(NULL),
      allowed_uid_(0) {
}

XWalkDevToolsServer::~XWalkDevToolsServer() {
  Stop();
}

// Allow connection from uid specified using AllowConnectionFromUid to devtools
// server. This supports the XDK usage: debug bridge wrapper runs in a separate
// process and connects to the devtools server.
bool XWalkDevToolsServer::CanUserConnectToDevTools(
    const net::UnixDomainServerSocket::Credentials& credentials) {
  if (credentials.user_id == allowed_uid_)
    return true;
  return content::CanUserConnectToDevTools(credentials);
}

void XWalkDevToolsServer::Start(bool allow_debug_permission) {
  if (protocol_handler_)
    return;

  net::UnixDomainServerSocket::AuthCallback auth_callback =
      allow_debug_permission ?
          base::Bind(&AuthorizeSocketAccessWithDebugPermission) :
          base::Bind(&content::CanUserConnectToDevTools);

  scoped_ptr<content::DevToolsHttpHandler::ServerSocketFactory> factory(
      new UnixDomainServerSocketFactory(socket_name_));
  protocol_handler_ = content::DevToolsHttpHandler::Start(
      factory.Pass(),
      base::StringPrintf(kFrontEndURL, content::GetWebKitRevision().c_str()),
      new XWalkDevToolsHttpHandlerDelegate(auth_callback), base::FilePath());
}

void XWalkDevToolsServer::Stop() {
  if (!protocol_handler_)
    return;
  // Note that the call to Stop() below takes care of |protocol_handler_|
  // deletion.
  protocol_handler_->Stop();
  protocol_handler_ = NULL;
}

bool XWalkDevToolsServer::IsStarted() const {
  return protocol_handler_;
}

void XWalkDevToolsServer::AllowConnectionFromUid(uid_t uid) {
  allowed_uid_ = uid;
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
                                      jboolean allow_debug_permission) {
  XWalkDevToolsServer* devtools_server =
      reinterpret_cast<XWalkDevToolsServer*>(server);
  if (enabled) {
    devtools_server->Start(allow_debug_permission);
  } else {
    devtools_server->Stop();
  }
}

static void AllowConnectionFromUid(JNIEnv* env,
                                    jobject obj,
                                    jlong server,
                                    jint uid) {
  XWalkDevToolsServer* devtools_server =
      reinterpret_cast<XWalkDevToolsServer*>(server);
  devtools_server->AllowConnectionFromUid((uid_t) uid);
}

}  // namespace xwalk
