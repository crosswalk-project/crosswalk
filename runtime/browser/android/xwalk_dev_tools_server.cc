// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_dev_tools_server.h"

#include <sys/types.h>
#include <unistd.h>

#include "base/android/jni_string.h"
#include "base/basictypes.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "content/public/browser/android/devtools_auth.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "grit/xwalk_resources.h"
#include "jni/XWalkDevToolsServer_jni.h"
#include "net/socket/unix_domain_socket_posix.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/chrome_version.h"

using content::DevToolsAgentHost;

namespace {

// FIXME(girish): The frontend URL needs to be served from the domain below
// for remote debugging to work in chrome (see chrome's devtools_ui.cc).
// Currently, the chrome version is hardcoded because of this dependancy.
const char kFrontEndURL[] =
    "http://chrome-devtools-frontend.appspot.com/static/%s/devtools.html";
const char kChromeVersion[] = CHROME_VERSION_STRING;

class Target : public content::DevToolsTarget {
 public:
  explicit Target() {
  }

  virtual std::string GetId() const OVERRIDE { return std::string(); }
  virtual std::string GetType() const OVERRIDE { return std::string(); }
  virtual std::string GetTitle() const OVERRIDE { return std::string(); }
  virtual std::string GetDescription() const OVERRIDE { return std::string(); }
  virtual GURL GetUrl() const OVERRIDE { return url_; }
  virtual GURL GetFaviconUrl() const OVERRIDE { return favicon_url_; }
  virtual base::TimeTicks GetLastActivityTime() const OVERRIDE {
    return last_activity_time_;
  }
  virtual bool IsAttached() const OVERRIDE { return false; }
  virtual scoped_refptr<DevToolsAgentHost> GetAgentHost() const OVERRIDE {
    return agent_host_;
  }
  virtual bool Activate() const OVERRIDE { return true; }
  virtual bool Close() const OVERRIDE { return true; }

 private:
  scoped_refptr<DevToolsAgentHost> agent_host_;
  GURL url_;
  GURL favicon_url_;
  base::TimeTicks last_activity_time_;
};

// Delegate implementation for the devtools http handler on android. A new
// instance of this gets created each time devtools is enabled.
class XWalkDevToolsServerDelegate
  : public content::DevToolsHttpHandlerDelegate {
 public:
  explicit XWalkDevToolsServerDelegate() {
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

  virtual std::string GetPageThumbnailData(const GURL& url) OVERRIDE {
    return std::string();
  }

  virtual scoped_ptr<content::DevToolsTarget> CreateNewTarget(
      const GURL&) OVERRIDE {
    return scoped_ptr<content::DevToolsTarget>(new Target());
  }

  virtual scoped_ptr<net::StreamListenSocket> CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) OVERRIDE {
    return scoped_ptr<net::StreamListenSocket>();
  }

  virtual void EnumerateTargets(TargetCallback callback) OVERRIDE {
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(XWalkDevToolsServerDelegate);
};

// Allow connection from same uid to devtools server.
// This supports the XDK usage: debug bridge wrapper connects to the devtools
// server.
static bool CanUserConnectToDevTools(uid_t uid, gid_t gid) {
  if (uid == getuid())
    return true;

  return content::CanUserConnectToDevTools(uid, gid);
}

}  // namespace

namespace xwalk {

XWalkDevToolsServer::XWalkDevToolsServer(const std::string& socket_name)
    : socket_name_(socket_name),
      protocol_handler_(NULL) {
}

XWalkDevToolsServer::~XWalkDevToolsServer() {
  Stop();
}

void XWalkDevToolsServer::Start() {
  if (protocol_handler_)
    return;

  protocol_handler_ = content::DevToolsHttpHandler::Start(
      new net::UnixDomainSocketWithAbstractNamespaceFactory(
          socket_name_,
          "",  // fallback socket name
          base::Bind(&CanUserConnectToDevTools)),
      base::StringPrintf(kFrontEndURL, kChromeVersion),
      new XWalkDevToolsServerDelegate());
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

bool RegisterXWalkDevToolsServer(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

static jint InitRemoteDebugging(JNIEnv* env,
                                jobject obj,
                                jstring socketName) {
  XWalkDevToolsServer* server = new XWalkDevToolsServer(
      base::android::ConvertJavaStringToUTF8(env, socketName));
  return reinterpret_cast<jint>(server);
}

static void DestroyRemoteDebugging(JNIEnv* env, jobject obj, jint server) {
  delete reinterpret_cast<XWalkDevToolsServer*>(server);
}

static jboolean IsRemoteDebuggingEnabled(JNIEnv* env,
                                         jobject obj,
                                         jint server) {
  return reinterpret_cast<XWalkDevToolsServer*>(server)->IsStarted();
}

static void SetRemoteDebuggingEnabled(JNIEnv* env,
                                      jobject obj,
                                      jint server,
                                      jboolean enabled) {
  XWalkDevToolsServer* devtools_server =
      reinterpret_cast<XWalkDevToolsServer*>(server);
  if (enabled) {
    devtools_server->Start();
  } else {
    devtools_server->Stop();
  }
}

}  // namespace xwalk
